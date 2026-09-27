/*
 * LIN Break & LINAUTO Diagnostic Probe (Optiboot_OL4 Bear-Metal Runtime Base)
 * Target: ADX Core-D (Microchip ATtiny1616-MNR, RS-485: SP485EEN)
 *
 * Hardware Pin Mapping:
 *   PA1: USART0 TXD (Alternate Pin via PORTMUX)
 *   PA2: USART0 RXD (Alternate Pin, LINAUTO Slave)
 *   PA3: RS-485 DE  (Driver Enable, Active HIGH via VPORTA)
 *   PA7: RS-485 /RE (Receiver Enable, Active LOW via VPORTA)
 *   PB2: Onboard RED LED (Status indicator, Active HIGH via VPORTB)
 */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay_basic.h>

#define BAUD_RATE 19200L
#define BAUD_SETTING_16 (((16000000UL / 6) * 64) / (16L * BAUD_RATE))
#define BAUD_SETTING_20 (((20000000UL / 6) * 64) / (16L * BAUD_RATE))

#define STATUS_OK 0x00

static uint16_t frame_count = 0;
static uint16_t isfif_count = 0;

static inline void rs485_tx_start(void) {
  VPORTA.OUT |= (1 << 3) | (1 << 7); // DE=1, /RE=1
}

static inline void putch(uint8_t ch) {
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  USART0.TXDATAL = ch;
}

static inline void rs485_tx_end(void) {
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm;
  VPORTA.OUT &= ~((1 << 3) | (1 << 7)); // DE=0, /RE=0

  // Critical Invariant: Re-arm Wait-For-Break (WFB) and clear all error flags.
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

  // Drain RX FIFO completely (read both H and L)
  while (USART0.STATUS & USART_RXCIF_bm) {
    (void)USART0.RXDATAH;
    (void)USART0.RXDATAL;
  }
}

static inline void response_space(void) {
  _delay_loop_1(150); // ~60us turnaround
}

static inline uint16_t crc16_update(uint16_t crc, uint8_t data) {
  crc ^= ((uint16_t)data << 8);
  for (uint8_t i = 0; i < 8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}

static inline void watchdogConfig(uint8_t x) {
  while (WDT.STATUS & WDT_SYNCBUSY_bm)
    ;
  _PROTECTED_WRITE(WDT.CTRLA, x);
}

// Header Reception: Waits for Break + Sync(0x55) + PID with WFB=1 and LED blinking
static uint8_t getch_header(void) {
  uint16_t loop = 0;

  while (!(USART0.STATUS & USART_RXCIF_bm)) {
    if (USART0.STATUS & USART_ISFIF_bm) {
      isfif_count++;
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    }

    if (++loop == 0) {
      VPORTB.IN |= (1 << 2); // Toggle RED LED (PB2) at ~2Hz heartbeat
      // Safe Guard: Periodically re-arm WFB so any corrupted sync state recovers immediately!
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    }
  }

  VPORTB.OUT &= ~(1 << 2); // LED OFF when data arrives

  (void)USART0.RXDATAH;
  uint8_t ch = USART0.RXDATAL;
  return ch;
}

int main(void) __attribute__((naked)) __attribute__((section(".init9")));
int main(void) {
  // Clear __zero_reg__ (r1)
  __asm__ __volatile__("clr r1\n\t");

  // Initialize Stack Pointer to top of SRAM (0x3FFF)
  SPL = 0xFF;
  SPH = 0x3F;

  // Disable Watchdog for diagnostic testing
  watchdogConfig(WDT_PERIOD_OFF_gc);

  // Pin Directions: PA1(TX)=OUT, PA3(DE)=OUT, PA7(/RE)=OUT, PB2(LED)=OUT
  VPORTA.DIR |= (1 << 1) | (1 << 3) | (1 << 7);
  VPORTA.OUT |= (1 << 1);               // PA1=HIGH
  VPORTA.OUT &= ~((1 << 3) | (1 << 7)); // DE=0, /RE=0 (Bus released)

  VPORTB.DIR |= (1 << 2);
  VPORTB.OUT &= ~(1 << 2);              // Red LED OFF

  // Route USART0 to alternate pins PA1(TXD) / PA2(RXD)
  PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

  // Initialize USART0 in LINAUTO mode (Hardware Break + 0x55 Auto-Calibration)
  if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
    USART0.BAUD = BAUD_SETTING_16;
  } else {
    USART0.BAUD = BAUD_SETTING_20;
  }
  USART0.DBGCTRL = 1;
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLA = 0;
  USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;

  // Arm Break detection
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

  for (;;) {
    // Wait for Break + Sync(0x55) + PID
    uint8_t ch = getch_header();
    frame_count++;
    uint16_t auto_baud = USART0.BAUD;

    // Send LN-485 Standard Diagnostic Packet:
    // [STATUS_OK, LEN=7, PID, frame_count(2B), isfif_count(2B), auto_baud(2B), CRC16(2B)]
    response_space();
    rs485_tx_start();

    putch(STATUS_OK);
    putch(0x07); // Length = 7

    uint16_t crc = 0xFFFF;

    putch(ch); // PID
    crc = crc16_update(crc, ch);

    uint8_t b;
    b = (uint8_t)(frame_count >> 8); putch(b); crc = crc16_update(crc, b);
    b = (uint8_t)(frame_count & 0xFF); putch(b); crc = crc16_update(crc, b);

    b = (uint8_t)(isfif_count >> 8); putch(b); crc = crc16_update(crc, b);
    b = (uint8_t)(isfif_count & 0xFF); putch(b); crc = crc16_update(crc, b);

    b = (uint8_t)(auto_baud >> 8); putch(b); crc = crc16_update(crc, b);
    b = (uint8_t)(auto_baud & 0xFF); putch(b); crc = crc16_update(crc, b);

    putch((uint8_t)(crc >> 8));
    putch((uint8_t)(crc & 0xFF));

    rs485_tx_end();
  }

  return 0;
}
