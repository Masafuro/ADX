/*
 * LIN Break & LINAUTO Diagnostic Probe (Dual-Channel Telemetry Edition)
 * Target: ADX Core-D (Microchip ATtiny1616-MNR)
 *
 * Pin Mapping:
 *   [Channel 1: RS-485 / LN-485 under Test - COM19]
 *     PA1: USART0 TXD (Alternate Pin via PORTMUX)
 *     PA2: USART0 RXD (Alternate Pin, LINAUTO Slave)
 *     PA3: RS-485 DE  (Driver Enable, Active HIGH via VPORTA)
 *     PA7: RS-485 /RE (Receiver Enable, Active LOW via VPORTA)
 *
 *   [Channel 2: CH342K Independent Debug Telemetry - COM21]
 *     PB4: Soft-UART TXD (9,600 bps, 8N1) -> PC COM21
 *     PB2: Onboard RED LED (Status indicator, Active HIGH via VPORTB)
 *
 *   [Channel 3: SerialUPDI Programming - COM20]
 *     PA0: UPDI Programming line
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

// =========================================================================
// Channel 2: CH342K Independent Soft-UART TX on PB4 @ 9,600 bps
// F_CPU = 3.333333 MHz (OSC20M / 6).
// 1 bit = 104.167 us = 347.2 cycles.
// _delay_loop_2(n) consumes 4 * n cycles.
// =========================================================================
static void dbg_putch(uint8_t c) {
  // Start bit: LOW
  VPORTB.OUT &= ~(1 << 4);
  _delay_loop_2(86); // 86 * 4 = 344 cycles (~103.2 us)

  // 8 Data bits (LSB first)
  for (uint8_t i = 0; i < 8; i++) {
    if (c & 0x01) {
      VPORTB.OUT |= (1 << 4);
    } else {
      VPORTB.OUT &= ~(1 << 4);
    }
    c >>= 1;
    _delay_loop_2(83); // 332 cycles + loop overhead (~15 cycles) ≒ 347 cycles
  }

  // Stop bit: HIGH
  VPORTB.OUT |= (1 << 4);
  _delay_loop_2(86); // 344 cycles (~103.2 us)
}

static void dbg_print(const char *str) {
  uint8_t safety = 128; // Guard against unterminated string
  while (*str && safety--) {
    dbg_putch((uint8_t)*str++);
  }
}

static void dbg_print_hex8(uint8_t val) {
  const char hex_chars[] = "0123456789ABCDEF";
  dbg_putch(hex_chars[(val >> 4) & 0x0F]);
  dbg_putch(hex_chars[val & 0x0F]);
}

static void dbg_print_hex16(uint16_t val) {
  dbg_print_hex8((uint8_t)(val >> 8));
  dbg_print_hex8((uint8_t)(val & 0xFF));
}

// =========================================================================
// Channel 1: RS-485 Half-Duplex Operations
// =========================================================================
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

  // Release bus: DE=0, /RE=0
  VPORTA.OUT &= ~((1 << 3) | (1 << 7));

  // Capture physical RXD pin level immediately after bus release:
  uint8_t rxd_after = (VPORTA.IN & (1 << 2)) ? 1 : 0;

  // Re-arm Wait-For-Break (WFB)
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

  // Drain RX FIFO
  while (USART0.STATUS & USART_RXCIF_bm) {
    (void)USART0.RXDATAH;
    (void)USART0.RXDATAL;
  }

  // Telemetry: Report post-transmission physical status to COM21
  dbg_print(" [TX_DONE] RXD=");
  dbg_putch(rxd_after ? '1' : '0');
  dbg_print(" ST=0x");
  dbg_print_hex8(USART0.STATUS);
  dbg_print("\r\n");
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

// =========================================================================
// Header Reception: Monitored with COM21 Evidence Telemetry
// =========================================================================
static uint8_t getch_header(void) {
  uint16_t hb_loop = 0;
  uint16_t idle_ticks = 0;

  for (;;) {
    // 1. Monitor ISFIF (Sync error)
    if (USART0.STATUS & USART_ISFIF_bm) {
      isfif_count++;
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
      dbg_print("[SYNC_ERR] ISFIF=1 BAUD=0x");
      dbg_print_hex16(USART0.BAUD);
      dbg_print(" ST=0x");
      dbg_print_hex8(USART0.STATUS);
      dbg_print("\r\n");
    }

    // 2. Frame Reception Complete (Break + Sync 0x55 + PID)
    if (USART0.STATUS & USART_RXCIF_bm) {
      VPORTB.OUT &= ~(1 << 2); // LED OFF

      (void)USART0.RXDATAH;
      uint8_t ch = USART0.RXDATAL;

      // Telemetry: Log successful frame reception to COM21
      dbg_print("[FRAME 0x");
      dbg_print_hex16(frame_count + 1);
      dbg_print("] PID=0x");
      dbg_print_hex8(ch);
      dbg_print(" BAUD=0x");
      dbg_print_hex16(USART0.BAUD);
      dbg_print(" ST=0x");
      dbg_print_hex8(USART0.STATUS);
      return ch;
    }

    // 3. Heartbeat LED & Periodic Idle Telemetry (~1Hz)
    if (++hb_loop == 0) {
      VPORTB.IN |= (1 << 2); // Toggle RED LED
      idle_ticks++;

      // Log idle state to COM21 every ~1.5 second (every 4th loop)
      if ((idle_ticks & 0x03) == 0) {
        dbg_print("[IDLE] ST=0x");
        dbg_print_hex8(USART0.STATUS);
        dbg_print(" RXD=");
        dbg_putch((VPORTA.IN & (1 << 2)) ? '1' : '0');
        dbg_print(" F=0x");
        dbg_print_hex16(frame_count);
        dbg_print(" ISF=0x");
        dbg_print_hex16(isfif_count);
        dbg_print("\r\n");
      }
    }
  }
}

// =========================================================================
// Main Entry Point
// =========================================================================
int main(void) __attribute__((naked)) __attribute__((section(".init9")));
int main(void) {
  // Clear __zero_reg__ (r1)
  __asm__ __volatile__("clr r1\n\t");

  // Initialize Stack Pointer to top of SRAM (0x3FFF)
  SPL = 0xFF;
  SPH = 0x3F;

  // Disable Watchdog for diagnostic testing
  watchdogConfig(WDT_PERIOD_OFF_gc);

  // Pin Directions:
  //   PA1(TX)=OUT, PA3(DE)=OUT, PA7(/RE)=OUT
  //   PB2(LED)=OUT, PB4(STX)=OUT
  VPORTA.DIR |= (1 << 1) | (1 << 3) | (1 << 7);
  VPORTA.OUT |= (1 << 1);               // PA1=HIGH
  VPORTA.OUT &= ~((1 << 3) | (1 << 7)); // DE=0, /RE=0 (Bus released)

  VPORTB.DIR |= (1 << 2) | (1 << 4);
  VPORTB.OUT &= ~(1 << 2);              // Red LED OFF
  VPORTB.OUT |= (1 << 4);               // PB4 (STX) = HIGH (Idle)

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

  // Initial Boot Telemetry to COM21 @ 9600 bps
  dbg_print("\r\n=== [BOOT] Core-D LN-485 Diagnostic Probe (9600bps Soft-UART) ===\r\n");
  dbg_print(" RSTFR=0x"); dbg_print_hex8(RSTCTRL.RSTFR);
  dbg_print(" BAUD=0x"); dbg_print_hex16(USART0.BAUD);
  dbg_print(" ST=0x"); dbg_print_hex8(USART0.STATUS);
  dbg_print(" RXD="); dbg_putch((VPORTA.IN & (1 << 2)) ? '1' : '0');
  dbg_print("\r\n===================================================================\r\n");

  for (;;) {
    // Wait for Break + Sync(0x55) + PID
    uint8_t ch = getch_header();
    frame_count++;
    uint16_t auto_baud = USART0.BAUD;

    // Send LN-485 Standard Diagnostic Packet over RS-485 (COM19)
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
