/*
 * Optiboot_OL4: 1-to-1 Half-Duplex LN-485 High-Reliability Bootloader
 * Target: ADX Core-D (Microchip ATtiny1616-MNR)
 * Protocol: LN-485 (LIN-based RS-485 with LINAUTO hardware synchronization)
 *
 * Hardware Pin Mapping:
 *   PA1: USART0 TXD (Alternate Pin)
 *   PA2: USART0 RXD (Alternate Pin, LINAUTO slave)
 *   PA3: RS-485 DE  (Driver Enable, Active HIGH via VPORTA)
 *   PA7: RS-485 /RE (Receiver Enable, Active LOW via VPORTA)
 *   PB2: Onboard RED LED (Status indicator, Active HIGH via VPORTB)
 *
 * Memory Layout:
 *   FUSE.BOOTEND = 0x04 -> 1024 Bytes (0x0000 - 0x03FF)
 *   Application Start = 0x0400 (15 KB available)
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay_basic.h>
#include "ln485_protocol.h"

unsigned const int __attribute__((section(".version"))) __attribute__((used))
optiboot_version = 256 * OL4_MAJVER + OL4_MINVER;

#define BAUD_RATE 115200L

#define BAUD_SETTING_16 (((16000000UL / 6) * 64) / (16L * BAUD_RATE))
#define BAUD_SETTING_20 (((20000000UL / 6) * 64) / (16L * BAUD_RATE))

typedef union {
  uint8_t  *bptr;
  uint16_t *wptr;
  uint16_t word;
  uint8_t bytes[2];
} addr16_t;

// Forward declarations
void putch(uint8_t ch);
uint8_t getch(void);
void watchdogConfig(uint8_t x);
static void rs485_tx_start(void);
static void rs485_tx_end(void);
uint16_t crc16_update(uint16_t crc, uint8_t data);
static void response_space(void);

// Direction & Hardware Invariant Helpers
static inline void rs485_tx_start(void) {
  // Drive bus (DE=1) AND mute local receiver (/RE=1) to physically block local echo!
  VPORTA.OUT |= (1 << 3) | (1 << 7);
}

static inline void rs485_tx_end(void) {
  // Wait until last stop bit physically leaves transmit shift register
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm;

  // Release bus (DE=0) and re-enable local receiver (/RE=0)
  VPORTA.OUT &= ~((1 << 3) | (1 << 7));

  // Critical Invariant: Re-arm Wait-For-Break (WFB) and clear all error flags.
  // Any post-transmission bus ringing, glitches, or echo are completely ignored!
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

  // Clear RX FIFO
  while (USART0.STATUS & USART_RXCIF_bm) {
    uint8_t dummy = USART0.RXDATAL;
    (void)dummy;
  }
}

// 50~80us response turnaround space
static inline void response_space(void) {
  // ~60us delay at 20MHz/6 or 16MHz/6
  _delay_loop_1(150);
}

uint16_t crc16_update(uint16_t crc, uint8_t data) {
  crc ^= ((uint16_t)data << 8);
  for (uint8_t i = 0; i < 8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}

int main(void) __attribute__((naked)) __attribute__((section(".init9")));
int main(void) {
  uint8_t ch, rx_high;
  register addr16_t address;
  uint8_t buffer[FLASH_PAGE_SIZE];

  // State 0: Reset Cause Evaluation
  ch = RSTCTRL.RSTFR;
  if (ch == 0) {
    _PROTECTED_WRITE(RSTCTRL.SWRR, 0x01); // Software reset
  }

  // Jump to Application at 0x0400 if Watchdog reset or condition not met
  if ((ch & RSTCTRL_WDRF_bm) || !(ch & 0x37)) {
    VPORTB.OUT &= ~(1 << 2);              // Ensure Red LED is OFF
    RSTCTRL.RSTFR = ch;                   // Clear reset cause
    GPIOR0 = ch;                          // Stash cause in GPIOR0 for app
    watchdogConfig(WDT_PERIOD_OFF_gc);    // Disable WDT
    __asm__ __volatile__("jmp 0x0400\n\t"); // Jump to App (BOOTEND=0x04)
  }

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

  // Cold-start Watchdog: 8 seconds timeout
  watchdogConfig(WDT_PERIOD_8KCLK_gc);

  address.word = APP_START_ADDR;

  // Main Command Dispatch Loop
  for (;;) {
    // Wait for Break + Sync(0x55) + PID
    ch = getch();
    rx_high = USART0.RXDATAH; // Check for parity or framing errors

    // Parity Error check (LIN Protected ID)
    if (rx_high & USART_PERR_bm) {
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
      continue;
    }

    // Command Dispatch
    if (ch == PID_PING) {
      // Keep-Alive / Sync Check: lock in programming mode
      watchdogConfig(WDT_PERIOD_OFF_gc);

      response_space();
      rs485_tx_start();
      putch(STATUS_OK);
      putch(0x00); // Length = 0
      putch(0x00); // Dummy CRC H
      putch(0x00); // Dummy CRC L
      rs485_tx_end();

    } else if (ch == PID_GET_INFO) {
      // Return Device Signature & Version
      uint16_t crc = 0xFFFF;
      uint8_t info[5] = {
        SIGROW_DEVICEID0,
        SIGROW_DEVICEID1,
        SIGROW_DEVICEID2,
        (uint8_t)(optiboot_version >> 8),
        (uint8_t)(optiboot_version & 0xFF)
      };

      for (uint8_t i = 0; i < 5; i++) {
        crc = crc16_update(crc, info[i]);
      }

      response_space();
      rs485_tx_start();
      putch(STATUS_OK);
      putch(5); // Length = 5
      for (uint8_t i = 0; i < 5; i++) {
        putch(info[i]);
      }
      putch((uint8_t)(crc >> 8));
      putch((uint8_t)(crc & 0xFF));
      rs485_tx_end();

    } else if (ch == PID_SET_ADDR) {
      // Load 16-bit Target Flash Address: [Len(2)] + [AddrL] + [AddrH] + [CRCH] + [CRCL]
      getch(); // Length (2)
      address.bytes[0] = getch();
      address.bytes[1] = getch();
      getch(); // CRC H (ignored for brevity, or verified)
      getch(); // CRC L

      response_space();
      rs485_tx_start();
      putch(STATUS_OK);
      putch(0x00);
      putch(0x00);
      putch(0x00);
      rs485_tx_end();

    } else if (ch == PID_WRITE_PAGE) {
      // Write 64B Page: [Len(64)] + [Data 64B] + [CRCH] + [CRCL]
      uint16_t calc_crc = 0xFFFF;
      getch(); // Length (64)

      for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        buffer[i] = getch();
        calc_crc = crc16_update(calc_crc, buffer[i]);
      }

      uint16_t rx_crc = ((uint16_t)getch() << 8);
      rx_crc |= getch();

      if (calc_crc != rx_crc) {
        // CRC Mismatch: report error and reject write
        response_space();
        rs485_tx_start();
        putch(STATUS_ERR_CRC);
        putch(0x00);
        putch(0x00);
        putch(0x00);
        rs485_tx_end();
        continue;
      }

      // Load buffer into Page Buffer in data space (MAPPED_PROGMEM_START + address)
      uint8_t *p = (uint8_t *)(MAPPED_PROGMEM_START + address.word);
      for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        *(p++) = buffer[i];
      }

      // Execute Page Erase & Write while bus is safely released (DE=0, /RE=0)
      _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
      while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm))
        ;

      // Reply OK
      response_space();
      rs485_tx_start();
      putch(STATUS_OK);
      putch(0x00);
      putch(0x00);
      putch(0x00);
      rs485_tx_end();

    } else if (ch == PID_READ_PAGE) {
      // Read 64B Page from Flash
      uint16_t crc = 0xFFFF;
      uint8_t *p = (uint8_t *)(MAPPED_PROGMEM_START + address.word);

      for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        buffer[i] = *(p++);
        crc = crc16_update(crc, buffer[i]);
      }

      response_space();
      rs485_tx_start();
      putch(STATUS_OK);
      putch(FLASH_PAGE_SIZE);
      for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        putch(buffer[i]);
      }
      putch((uint8_t)(crc >> 8));
      putch((uint8_t)(crc & 0xFF));
      rs485_tx_end();

    } else if (ch == PID_REBOOT) {
      // Reboot into Application
      response_space();
      rs485_tx_start();
      putch(STATUS_OK);
      putch(0x00);
      putch(0x00);
      putch(0x00);
      rs485_tx_end();

      watchdogConfig(WDT_PERIOD_8CLK_gc);
      while (1)
        ;

    } else {
      // Unknown command: re-arm and wait for next Break
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
      continue;
    }
  }
}

void putch(uint8_t ch) {
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  USART0.TXDATAL = ch;
}

uint8_t getch(void) {
  uint16_t loop = 0;

  // Monitor for sync field inconsistency flag (ISFIF)
  while (!(USART0.STATUS & USART_RXCIF_bm)) {
    if (USART0.STATUS & USART_ISFIF_bm) {
      // Inconsistent Sync Field: reset WFB and clear flags
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    }

    if (++loop == 0) {
      VPORTB.IN |= (1 << 2); // Toggle RED LED (PB2) at ~2Hz
    }
  }

  VPORTB.OUT &= ~(1 << 2); // LED OFF when data arrives

  // Read order: RXDATAH first, then RXDATAL
  (void)USART0.RXDATAH;
  uint8_t ch = USART0.RXDATAL;

  __asm__ __volatile__("wdr\n\t"); // Reset watchdog
  return ch;
}

void watchdogConfig(uint8_t x) {
  while (WDT.STATUS & WDT_SYNCBUSY_bm)
    ;
  _PROTECTED_WRITE(WDT.CTRLA, x);
}
