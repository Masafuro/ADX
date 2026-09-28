/*
 * Simple RS-485 Byte Echo Benchmark (Pure Hardware Sanity Check)
 * Target: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)
 *
 * Pin Mapping:
 *   [Channel 1: RS-485 under Test - COM19]
 *     PA1: USART0 TXD (Alternate Pin via PORTMUX)
 *     PA2: USART0 RXD (Alternate Pin)
 *     PA4: RS-485 DE  (Driver Enable, Active HIGH via VPORTA)
 *     PA7: RS-485 /RE (Receiver Enable, Active LOW via VPORTA)
 *
 *   [Channel 2: Debug Telemetry - COM21]
 *     PB4: Soft-UART TXD (9,600 bps, 8N1) -> PC COM21
 *     PB2: Onboard RED LED (Blinks on RS-485 activity)
 */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay_basic.h>

#define BAUD_RATE 9600L
#define BAUD_SETTING_16 (((16000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1111
#define BAUD_SETTING_20 (((20000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1389

static uint16_t echo_count = 0;

// =========================================================================
// Channel 2: CH342K Independent Soft-UART TX on PB4 @ 9,600 bps
// =========================================================================
static void dbg_putch(uint8_t c) {
  // Start bit: LOW
  VPORTB.OUT &= ~(1 << 4);
  _delay_loop_2(86); // ~103.2 us

  for (uint8_t i = 0; i < 8; i++) {
    if (c & 0x01) {
      VPORTB.OUT |= (1 << 4);
    } else {
      VPORTB.OUT &= ~(1 << 4);
    }
    c >>= 1;
    _delay_loop_2(83); // ~347 cycles total
  }

  // Stop bit: HIGH
  VPORTB.OUT |= (1 << 4);
  _delay_loop_2(86);
}

static void dbg_print(const char *str) {
  uint8_t safety = 128;
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
// Channel 1: RS-485 Hardware Echo Operations
// =========================================================================
static inline void set_tx_mode(void) {
  VPORTA.OUT |= (1 << 4) | (1 << 7); // DE=1, /RE=1 (Mute local receiver)
  _delay_loop_2(830); // ~1ms transceiver mode switch delay (matching historical working test)
}

static inline void set_rx_mode(void) {
  _delay_loop_2(830); // ~1ms bus release settle delay
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0 (Enable local receiver)
}

static inline void rs485_putch(uint8_t ch) {
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  USART0.TXDATAL = ch;
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm;
}

// =========================================================================
// Main Entry Point
// =========================================================================
int main(void) __attribute__((naked)) __attribute__((section(".init9")));
int main(void) {
  // Clear r1 (__zero_reg__) and init Stack Pointer
  __asm__ __volatile__("clr r1\n\t");
  SPL = 0xFF;
  SPH = 0x3F;

  // Pin Directions:
  //   PA1(TX)=OUT, PA4(DE)=OUT, PA7(/RE)=OUT
  //   PB2(LED)=OUT, PB4(STX)=OUT
  // Note: PA3 is EXTCLK (untouched)
  VPORTA.DIR |= (1 << 1) | (1 << 4) | (1 << 7);
  VPORTA.OUT |= (1 << 1);               // PA1=HIGH
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0 (Start in RX mode)

  VPORTB.DIR |= (1 << 2) | (1 << 4);
  VPORTB.OUT &= ~(1 << 2);              // Red LED OFF
  VPORTB.OUT |= (1 << 4);               // PB4 (STX) = HIGH

  // Route USART0 to alternate pins PA1(TXD) / PA2(RXD)
  PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

  // Standard Asynchronous UART 9,600 bps, 8N1
  if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
    USART0.BAUD = BAUD_SETTING_16;
  } else {
    USART0.BAUD = BAUD_SETTING_20;
  }
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLA = 0;
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // Standard Asynchronous Normal Mode

  // Drain any startup garbage
  while (USART0.STATUS & USART_RXCIF_bm) {
    (void)USART0.RXDATAH;
    (void)USART0.RXDATAL;
  }

  // Initial Announcement on COM21
  dbg_print("\r\n=======================================================\r\n");
  dbg_print(" [BOOT] Core-D Simple RS-485 Byte Echo Probe v1.0\r\n");
  dbg_print(" USART0: Normal UART @ 9600 bps (BAUD=0x");
  dbg_print_hex16(USART0.BAUD);
  dbg_print(")\r\n");
  dbg_print(" DE=PA4, /RE=PA7 (Initialized to RX Mode)\r\n");
  dbg_print(" Waiting for bytes on RS-485 (COM19)...\r\n");
  dbg_print("=======================================================\r\n");

  uint16_t hb_loop = 0;
  uint16_t idle_ticks = 0;

  for (;;) {
    // 1. Check for incoming byte on RS-485
    if (USART0.STATUS & USART_RXCIF_bm) {
      (void)USART0.RXDATAH;
      uint8_t c = USART0.RXDATAL;
      echo_count++;

      // Turn on Red LED
      VPORTB.OUT |= (1 << 2);

      // Report reception to COM21 immediately
      dbg_print("[ECHO #");
      dbg_print_hex16(echo_count);
      dbg_print("] Recv: 0x");
      dbg_print_hex8(c);
      dbg_print(" ('");
      dbg_putch((c >= 32 && c <= 126) ? c : '.');
      dbg_print("') -> Echoing\r\n");

      // Echo back over RS-485
      set_tx_mode();
      rs485_putch(c);
      set_rx_mode();

      // Turn off Red LED
      VPORTB.OUT &= ~(1 << 2);
    }

    // 2. Periodic Idle heartbeat on COM21 (~1Hz)
    if (++hb_loop == 0) {
      idle_ticks++;
      if ((idle_ticks & 0x03) == 0) {
        dbg_print("[IDLE 9600] PA2_pin=");
        dbg_putch((VPORTA.IN & (1 << 2)) ? '1' : '0');
        dbg_print(" EchoCnt=0x");
        dbg_print_hex16(echo_count);
        dbg_print("\r\n");
      }
    }
  }

  return 0;
}
