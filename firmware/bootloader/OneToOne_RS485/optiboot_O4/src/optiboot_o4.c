/*
 * Optiboot_O4: 1-to-1 Half-Duplex RS-485 High-Reliability Bootloader
 * Target: ADX Core-D (Microchip ATtiny1616-MNR)
 * Protocol: STK500v1 Subset (115200 bps, 8N1)
 *
 * Hardware Pin Mapping:
 *   PA1: USART0 TXD
 *   PA2: USART0 RXD
 *   PA3: RS-485 DE  (Driver Enable, Active HIGH, software GPIO via VPORTA)
 *   PA7: RS-485 /RE (Receiver Enable, Active LOW, software GPIO via VPORTA)
 *   PB2: Onboard RED LED (Status indicator, Active HIGH via VPORTB)
 *
 * Direction Control Invariants:
 *   - Idle / Receiving: DE=0, /RE=0 (Bus released, receiver active)
 *   - Transmitting:     DE=1, /RE=1 (Receiver MUTED to physically block local echo!)
 *   - Post-Transmit:    Wait TXCIF -> DE=0, /RE=0 -> Flush RX FIFO residue
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "stk500.h"

#define OPTIBOOT_MAJVER 25
#define OPTIBOOT_MINVER 1

unsigned const int __attribute__((section(".version"))) __attribute__((used))
optiboot_version = 256 * OPTIBOOT_MAJVER + OPTIBOOT_MINVER;

#define BAUD_RATE 115200L

// Prescaled by 6 at reset: 16MHz/6 or 20MHz/6
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
void verifySpace(void);
void watchdogConfig(uint8_t x);
static void rs485_tx_start(void);
static void rs485_tx_end(void);

// Direction Control Functions
static inline void rs485_tx_start(void) {
  // Drive bus (DE=1) AND mute local receiver (/RE=1) to prevent self-echo!
  VPORTA.OUT |= (1 << 3) | (1 << 7);
}

static inline void rs485_tx_end(void) {
  // Wait until the last stop bit has physically cleared the transmit shift register
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm; // Clear TXCIF flag

  // Release bus (DE=0) and re-enable local receiver (/RE=0)
  VPORTA.OUT &= ~((1 << 3) | (1 << 7));

  // Flush any possible residual data or glitch from the RX FIFO
  while (USART0.STATUS & USART_RXCIF_bm) {
    uint8_t dummy = USART0.RXDATAL;
    (void)dummy;
  }
}

int main(void) __attribute__((naked)) __attribute__((section(".init9")));
int main(void) {
  uint8_t ch;
  register addr16_t address;
  register uint8_t length;

  // State 0: Reset Cause Evaluation
  ch = RSTCTRL.RSTFR;
  if (ch == 0) {
    _PROTECTED_WRITE(RSTCTRL.SWRR, 0x01); // Issue clean software reset
  }

  // If reset was caused by Watchdog (WDRF = bit 3) or entry conditions not met:
  // Jump straight to application at 0x0200
  if ((ch & RSTCTRL_WDRF_bm) || !(ch & 0x37)) {
    VPORTB.OUT &= ~(1 << 2);              // Ensure Red LED is OFF
    RSTCTRL.RSTFR = ch;                   // Clear reset cause
    GPIOR0 = ch;                          // Stash cause in GPIOR0 for application
    watchdogConfig(WDT_PERIOD_OFF_gc);    // Disable WDT
    __asm__ __volatile__("jmp 0x0200\n\t"); // Jump to App
  }

  // Initialize GPIO Directions
  // PA1(TX)=OUT, PA3(DE)=OUT, PA7(/RE)=OUT
  VPORTA.DIR |= (1 << 1) | (1 << 3) | (1 << 7);
  // PA1=HIGH (UART idle), DE=0, /RE=0 (Receiver active, bus released)
  VPORTA.OUT |= (1 << 1);
  VPORTA.OUT &= ~((1 << 3) | (1 << 7));

  // PB2(Red LED)=OUT, LOW
  VPORTB.DIR |= (1 << 2);
  VPORTB.OUT &= ~(1 << 2);

  // Initialize USART0 (Standard Asynchronous, no interrupts, no hardware XDIR)
  if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
    USART0.BAUD = BAUD_SETTING_16;
  } else {
    USART0.BAUD = BAUD_SETTING_20;
  }
  USART0.DBGCTRL = 1;                                        // Run during debug
  USART0.CTRLC = USART_CHSIZE_gm & USART_CHSIZE_8BIT_gc;     // 8N1
  USART0.CTRLA = 0;                                          // Standard mode
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;              // Enable RX & TX

  // State 1: Start 8s watchdog for cold-start wait
  watchdogConfig(WDT_PERIOD_8KCLK_gc); // 8 seconds timeout

  // Main Command Dispatch Loop
  for (;;) {
    ch = getch();

    if (ch == STK_GET_PARAMETER) {
      uint8_t which = getch();
      verifySpace();
      if (which == STK_SW_MINOR) {
        putch(optiboot_version & 0xFF);
      } else if (which == STK_SW_MAJOR) {
        putch(optiboot_version >> 8);
      } else {
        putch(0x03);
      }
    } else if (ch == STK_GET_SYNC) {
      verifySpace();
    } else if (ch == STK_ENTER_PROGMODE) {
      // State 2: Lock into programming mode unconditionally
      // Disable watchdog completely so verification / programming never times out!
      watchdogConfig(WDT_PERIOD_OFF_gc);
      verifySpace();
    } else if (ch == STK_LEAVE_PROGMODE) {
      // State 4: Clean reboot into application
      verifySpace();
      putch(STK_OK);
      rs485_tx_end();
      // Arm 8ms watchdog and spin-lock so hardware resets cleanly
      watchdogConfig(WDT_PERIOD_8CLK_gc);
      while (1)
        ;
    } else if (ch == STK_LOAD_ADDRESS) {
      address.bytes[0] = getch();
      address.bytes[1] = getch();
      verifySpace();
    } else if (ch == STK_READ_SIGN) {
      verifySpace();
      putch(SIGROW_DEVICEID0);
      putch(SIGROW_DEVICEID1);
      putch(SIGROW_DEVICEID2);
    } else if (ch == STK_PROG_PAGE) {
      // Program Flash Page (64 bytes)
      getch(); // length high (ignored, assume 64)
      length = getch(); // length low
      getch(); // desttype ('F')

      address.word += MAPPED_PROGMEM_START;

      do {
        *(address.bptr++) = getch();
      } while (--length);

      verifySpace(); // Sends STK_INSYNC and starts RS-485 TX

      // Issue Page Erase & Write command to NVM controller
      _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
      while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm))
        ;
    } else if (ch == STK_READ_PAGE) {
      // Read Flash Page (64 bytes)
      getch(); // length high
      length = getch(); // length low
      getch(); // desttype ('F')

      verifySpace(); // Sends STK_INSYNC and starts RS-485 TX

      address.word += MAPPED_PROGMEM_START;

      do {
        putch(*(address.bptr++));
      } while (--length);
    } else {
      // Unknown / unsupported command: ignore payload until CRC_EOP
      continue;
    }

    // Common response completion for all commands
    putch(STK_OK);
    rs485_tx_end();
  }
}

void putch(uint8_t ch) {
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  USART0.TXDATAL = ch;
}

uint8_t getch(void) {
  uint8_t ch, flags;
  uint16_t loop = 0;
  uint8_t loop_h = 0;

  while (!(USART0.STATUS & USART_RXCIF_bm)) {
    if (++loop == 0) {
      if (++loop_h >= 4) {
        loop_h = 0;
        VPORTB.IN |= (1 << 2); // Toggle RED LED (PB2)
      }
    }
  }

  VPORTB.OUT &= ~(1 << 2); // Turn off RED LED when byte arrives
  flags = USART0.RXDATAH;
  ch = USART0.RXDATAL;

  if ((flags & USART_FERR_bm) == 0) {
    __asm__ __volatile__("wdr\n\t"); // Reset watchdog on clean byte
  }

  return ch;
}

void verifySpace(void) {
  if (getch() != CRC_EOP) {
    // Communication framing error: reset to application via fast watchdog
    watchdogConfig(WDT_PERIOD_8CLK_gc);
    while (1)
      ;
  }
  // Turn on RS-485 transmitter and send sync response
  rs485_tx_start();
  putch(STK_INSYNC);
}

void watchdogConfig(uint8_t x) {
  while (WDT.STATUS & WDT_SYNCBUSY_bm)
    ;
  _PROTECTED_WRITE(WDT.CTRLA, x);
}
