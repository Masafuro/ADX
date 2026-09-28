/*
 * Demo User Application for ADX Core-D (ATtiny1616)
 * Located at 0x0400 (BOOTEND = 0x04).
 *
 * Behavior:
 *   - PB2 (Red LED): Blinks periodically at ~1Hz (500ms ON / 500ms OFF)
 *   - PB4 (Soft-UART): Prints heartbeat messages to COM21 @ 9600 bps
 */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay_basic.h>

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

static void dbg_print_hex16(uint16_t val) {
  const char hex_chars[] = "0123456789ABCDEF";
  dbg_putch(hex_chars[(val >> 12) & 0x0F]);
  dbg_putch(hex_chars[(val >> 8) & 0x0F]);
  dbg_putch(hex_chars[(val >> 4) & 0x0F]);
  dbg_putch(hex_chars[val & 0x0F]);
}

void _start(void) __attribute__((naked)) __attribute__((section(".text")));
void _start(void) {
  __asm__ __volatile__("clr r1\n\t");
  SPL = 0xFF;
  SPH = 0x3F;

  // PB2(LED)=OUT, PB4(Soft-UART TX)=OUT
  VPORTB.DIR |= (1 << 2) | (1 << 4);
  VPORTB.OUT &= ~(1 << 2); // LED OFF
  VPORTB.OUT |= (1 << 4);  // PB4=HIGH

  dbg_print("\r\n=======================================================\r\n");
  dbg_print(" [SUCCESS] User Application running at 0x0400!\r\n");
  dbg_print(" ADX Core-D Firmware launched by Optiboot_OL4 v2.0\r\n");
  dbg_print("=======================================================\r\n");

  uint16_t tick = 0;

  for (;;) {
    // Toggle Red LED
    VPORTB.IN |= (1 << 2);

    // Heartbeat message
    dbg_print("[APP HEARTBEAT] Tick=0x");
    dbg_print_hex16(++tick);
    dbg_print(" (LED Toggled)\r\n");

    // ~500ms delay at 3.33MHz
    for (uint8_t i = 0; i < 50; i++) {
      _delay_loop_2(8300); // ~10ms
    }
  }
}
