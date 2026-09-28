/*
 * RS-485 Interactive User Application for ADX Core-D (ATtiny1616)
 * Located at 0x0400 (Flashed via Optiboot_OL4 v2.0 over RS-485!)
 *
 * Hardware Pin Mapping:
 *   PA1: USART0 TXD (Alternate Pin via PORTMUX)
 *   PA2: USART0 RXD (Alternate Pin)
 *   PA4: RS-485 DE  (Driver Enable, Active HIGH via VPORTA)
 *   PA7: RS-485 /RE (Receiver Enable, Active LOW via VPORTA)
 *   PB2: Onboard RED LED (Blinks on RS-485 RX/TX)
 *   PB4: Soft-UART TXD (Debug Telemetry -> COM21 @ 9600 bps)
 *
 * Behavior:
 *   1. Prints banner on RS-485 (COM19) and Soft-UART (COM21) upon boot.
 *   2. Any character received on RS-485 (COM19) is echoed back with "[ECHO] 'c'".
 *   3. Periodically broadcasts a heartbeat message on RS-485 (~2s interval).
 */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay_basic.h>

#define BAUD_RATE 9600L
#define BAUD_SETTING_16 (((16000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1111 (0x0457)
#define BAUD_SETTING_20 (((20000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1389 (0x056D)

// =========================================================================
// Channel 2: COM21 Soft-UART Telemetry (PB4)
// =========================================================================
static void dbg_putch(uint8_t c) {
  VPORTB.OUT &= ~(1 << 4);
  _delay_loop_2(86); // ~103.2 us

  for (uint8_t i = 0; i < 8; i++) {
    if (c & 0x01) {
      VPORTB.OUT |= (1 << 4);
    } else {
      VPORTB.OUT &= ~(1 << 4);
    }
    c >>= 1;
    _delay_loop_2(83);
  }

  VPORTB.OUT |= (1 << 4);
  _delay_loop_2(86);
}

static void dbg_print(const char *str) {
  while (*str) {
    dbg_putch((uint8_t)*str++);
  }
}

static void dbg_print_dec(uint16_t val) {
  char buf[6];
  int idx = 0;
  if (val == 0) {
    dbg_putch('0');
    return;
  }
  while (val > 0) {
    buf[idx++] = '0' + (val % 10);
    val /= 10;
  }
  while (idx > 0) {
    dbg_putch(buf[--idx]);
  }
}

// =========================================================================
// Channel 1: RS-485 Hardware UART (COM19)
// =========================================================================
static inline void rs485_set_tx(void) {
  VPORTA.OUT |= (1 << 4) | (1 << 7); // DE=1, /RE=1
  _delay_loop_2(830);                // ~1ms setup delay
}

static inline void rs485_set_rx(void) {
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm;
  _delay_loop_2(830);                // ~1ms settle delay
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0
}

static void rs485_putch(uint8_t ch) {
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  USART0.TXDATAL = ch;
}

static void rs485_print(const char *str) {
  rs485_set_tx();
  while (*str) {
    rs485_putch((uint8_t)*str++);
  }
  rs485_set_rx();
}

static void rs485_print_dec(uint16_t val) {
  char buf[6];
  int idx = 0;
  if (val == 0) {
    rs485_putch('0');
    return;
  }
  while (val > 0) {
    buf[idx++] = '0' + (val % 10);
    val /= 10;
  }
  while (idx > 0) {
    rs485_putch(buf[--idx]);
  }
}

// =========================================================================
// Application Entry Point at 0x0400
// =========================================================================
void _start(void) __attribute__((naked)) __attribute__((section(".text")));
void _start(void) {
  // Clear r1 and setup stack pointer
  __asm__ __volatile__("clr r1\n\t");
  SPL = 0xFF;
  SPH = 0x3F;

  // Pin Directions:
  //   PA1(TX)=OUT, PA4(DE)=OUT, PA7(/RE)=OUT
  //   PB2(LED)=OUT, PB4(Soft-UART TX)=OUT
  VPORTA.DIR |= (1 << 1) | (1 << 4) | (1 << 7);
  VPORTA.OUT |= (1 << 1);               // PA1=HIGH
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // Start in RX Mode

  VPORTB.DIR |= (1 << 2) | (1 << 4);
  VPORTB.OUT &= ~(1 << 2);              // LED OFF
  VPORTB.OUT |= (1 << 4);               // PB4=HIGH

  // Route USART0 to alternate pins PA1/PA2
  PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

  // Configure USART0 for 9,600 bps Normal Asynchronous UART
  if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
    USART0.BAUD = BAUD_SETTING_16;
  } else {
    USART0.BAUD = BAUD_SETTING_20;
  }
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLA = 0;
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;

  // Flush buffer
  while (USART0.STATUS & USART_RXCIF_bm) {
    (void)USART0.RXDATAH;
    (void)USART0.RXDATAL;
  }

  // 1. Send Banner over RS-485 (COM19) and COM21
  rs485_print("\r\n=======================================================\r\n");
  rs485_print(" [SUCCESS] ADX Core-D RS-485 User Application Running!\r\n");
  rs485_print(" Executing at 0x0400 (Flashed via Optiboot_OL4 v2.0)\r\n");
  rs485_print(" Baud: 9600 bps, 8N1 | RS-485 DE=PA4, /RE=PA7\r\n");
  rs485_print(" Type any character or send text to test RS-485 echo!\r\n");
  rs485_print("=======================================================\r\n");

  dbg_print("[APP BOOT] User application running at 0x0400!\r\n");

  uint16_t heartbeat_tick = 0;
  uint16_t loop_counter = 0;

  for (;;) {
    // 1. Check for incoming data on RS-485 (COM19)
    if (USART0.STATUS & USART_RXCIF_bm) {
      (void)USART0.RXDATAH;
      uint8_t c = USART0.RXDATAL;

      // Toggle Red LED to indicate activity
      VPORTB.IN |= (1 << 2);

      // Echo back with prefix over RS-485 (COM19)
      rs485_set_tx();
      rs485_print("[Core-D Echo] '");
      rs485_putch(c);
      rs485_print("'\r\n");
      rs485_set_rx();

      // Log to COM21
      dbg_print("[RS-485 RX] Char: '");
      dbg_putch(c);
      dbg_print("'\r\n");

      loop_counter = 0; // Reset heartbeat loop
    }

    // 2. Periodic Heartbeat Broadcast on RS-485 (~2.0 seconds)
    if (++loop_counter > 50000) {
      loop_counter = 0;
      heartbeat_tick++;

      // Blink LED
      VPORTB.IN |= (1 << 2);

      // Send periodic heartbeat message on RS-485 (COM19)
      rs485_set_tx();
      rs485_print("[Core-D RS-485 Heartbeat #");
      rs485_print_dec(heartbeat_tick);
      rs485_print("] Application alive at 0x0400!\r\n");
      rs485_set_rx();

      dbg_print("[APP HEARTBEAT] Tick #");
      dbg_print_dec(heartbeat_tick);
      dbg_print("\r\n");
    }

    _delay_loop_1(100); // Small loop pacing (~30us)
  }
}
