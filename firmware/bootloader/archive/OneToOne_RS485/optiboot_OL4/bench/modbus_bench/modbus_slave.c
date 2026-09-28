/*
 * MODBUS-RTU Style High-Reliability Ping-Pong Slave Benchmark
 * Target: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)
 *
 * Communication:
 *   - Channel 1 (RS-485 COM19): Standard Asynchronous UART @ 19,200 bps (8N1)
 *     PA1: USART0 TXD (Alternate Pin via PORTMUX)
 *     PA2: USART0 RXD (Alternate Pin)
 *     PA4: RS-485 DE  (Driver Enable, Active HIGH via VPORTA)
 *     PA7: RS-485 /RE (Receiver Enable, Active LOW via VPORTA)
 *
 *   - Channel 2 (Debug COM21): PB4 Soft-UART TX @ 9,600 bps (8N1)
 *     PB4: CH342K Port B Debug Telemetry
 *     PB2: Onboard RED LED
 */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay_basic.h>

#define SLAVE_ADDRESS 0x01
#define CMD_PING      0x03

#define BAUD_RATE 19200L
#define BAUD_SETTING (((20000000UL / 6) * 64) / (16L * BAUD_RATE)) // 694 for 20MHz/6

static uint16_t transaction_count = 0;

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
// MODBUS RTU CRC-16 Calculation (Polynomial 0xA001, Init 0xFFFF)
// =========================================================================
static uint16_t calc_crc16(const uint8_t *buf, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= (uint16_t)buf[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// =========================================================================
// Channel 1: RS-485 Operations (Standard Asynchronous UART)
// =========================================================================
static inline void rs485_tx_start(void) {
  VPORTA.OUT |= (1 << 4) | (1 << 7); // DE(PA4)=1, /RE(PA7)=1 (Mute receiver)
  _delay_loop_1(50); // ~20us transceiver turn-on time
}

static inline void rs485_putch(uint8_t ch) {
  uint16_t timeout = 50000;
  while (!(USART0.STATUS & USART_DREIF_bm) && --timeout)
    ;
  USART0.TXDATAL = ch;
}

static inline void rs485_tx_end(void) {
  uint16_t timeout = 50000;
  while (!(USART0.STATUS & USART_TXCIF_bm) && --timeout)
    ;
  USART0.STATUS = USART_TXCIF_bm;

  // Release bus: DE(PA4)=0, /RE(PA7)=0
  VPORTA.OUT &= ~((1 << 4) | (1 << 7));
  _delay_loop_1(50); // ~20us bus release settle
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

  // Pin Directions: PA1(TX)=OUT, PA4(DE)=OUT, PA7(/RE)=OUT, PB2(LED)=OUT, PB4(STX)=OUT
  // Note: PA3 is EXTCLK (do not configure as GPIO output!)
  VPORTA.DIR |= (1 << 1) | (1 << 4) | (1 << 7);
  VPORTA.OUT |= (1 << 1);               // PA1=HIGH
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0 (Bus released)

  VPORTB.DIR |= (1 << 2) | (1 << 4);
  VPORTB.OUT &= ~(1 << 2);              // Red LED OFF
  VPORTB.OUT |= (1 << 4);               // PB4 (STX) = HIGH

  // Route USART0 to alternate pins PA1(TXD) / PA2(RXD)
  PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

  // Standard Asynchronous UART 19,200 bps, 8N1 (NO LIN, NO Auto-Baud)
  USART0.BAUD = BAUD_SETTING;
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLA = 0;
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // Standard Normal UART

  // Drain any initial bytes
  while (USART0.STATUS & USART_RXCIF_bm) {
    (void)USART0.RXDATAH;
    (void)USART0.RXDATAL;
  }

  // Initialize variables
  transaction_count = 0;

  // Initial Boot Announcement
  dbg_print("\r\n===================================================\r\n");
  dbg_print(" [BOOT] Core-D MODBUS-RTU Bench Slave v1.1\r\n");
  dbg_print(" USART0: Normal UART @ 19,200 bps (BAUD=");
  dbg_print_hex16(USART0.BAUD);
  dbg_print(")\r\n===================================================\r\n");

  uint8_t rx_buf[16];
  uint8_t rx_len = 0;
  uint16_t idle_loop = 0;
  uint16_t idle_ticks = 0;

  for (;;) {
    // 1. Check for incoming byte
    if (USART0.STATUS & USART_RXCIF_bm) {
      (void)USART0.RXDATAH;
      rx_buf[0] = USART0.RXDATAL;
      rx_len = 1;

      // Receive remaining 5 bytes with 20ms per-byte timeout (resilient to USB jitter)
      while (rx_len < 6) {
        uint16_t timeout = 12000; // ~20ms timeout
        while (!(USART0.STATUS & USART_RXCIF_bm) && --timeout) {
          _delay_loop_1(5);
        }
        if (timeout == 0) {
          break; // Timeout!
        }
        (void)USART0.RXDATAH;
        rx_buf[rx_len++] = USART0.RXDATAL;
      }

      // 2. Validate Frame: Master Request should be 6 bytes:
      //    [ADDR=0x01] [CMD=0x03] [SEQ_H] [SEQ_L] [CRC_L] [CRC_H]
      if (rx_len == 6) {
        if (rx_buf[0] == SLAVE_ADDRESS && rx_buf[1] == CMD_PING) {
          uint16_t calc_crc = calc_crc16(rx_buf, 4);
          uint16_t recv_crc = rx_buf[4] | ((uint16_t)rx_buf[5] << 8);

          if (calc_crc == recv_crc) {
            transaction_count++;
            uint16_t seq = ((uint16_t)rx_buf[2] << 8) | rx_buf[3];

            VPORTB.OUT |= (1 << 2); // Toggle LED ON

            // Build Response Packet (8 bytes):
            // [0x01] [0x03] [SEQ_H] [SEQ_L] [COUNT_H] [COUNT_L] [CRC_L] [CRC_H]
            uint8_t tx_buf[8];
            tx_buf[0] = SLAVE_ADDRESS;
            tx_buf[1] = CMD_PING;
            tx_buf[2] = rx_buf[2];
            tx_buf[3] = rx_buf[3];
            tx_buf[4] = (uint8_t)(transaction_count >> 8);
            tx_buf[5] = (uint8_t)(transaction_count & 0xFF);

            uint16_t resp_crc = calc_crc16(tx_buf, 6);
            tx_buf[6] = (uint8_t)(resp_crc & 0xFF);
            tx_buf[7] = (uint8_t)(resp_crc >> 8);

            // Turnaround delay (~100us)
            _delay_loop_2(83);

            // Send response over RS-485
            rs485_tx_start();
            for (uint8_t i = 0; i < 8; i++) {
              rs485_putch(tx_buf[i]);
            }
            rs485_tx_end();

            VPORTB.OUT &= ~(1 << 2); // LED OFF

            // Debug Telemetry on COM21
            dbg_print("[MODBUS PASS] SEQ=0x");
            dbg_print_hex16(seq);
            dbg_print(" COUNT=0x");
            dbg_print_hex16(transaction_count);
            dbg_print("\r\n");
          } else {
            dbg_print("[MODBUS ERR] CRC mismatch!\r\n");
          }
        }
      } else {
        dbg_print("[MODBUS ERR] Incomplete len=");
        dbg_print_hex8(rx_len);
        dbg_print("\r\n");
      }

      // Reset buffer for next frame
      rx_len = 0;
    }

    // 3. Heartbeat IDLE telemetry (~1Hz)
    if (++idle_loop == 0) {
      idle_ticks++;
      if ((idle_ticks & 0x03) == 0) {
        dbg_print("[IDLE] ST=0x");
        dbg_print_hex8(USART0.STATUS);
        dbg_print(" COUNT=0x");
        dbg_print_hex16(transaction_count);
        dbg_print("\r\n");
      }
    }
  }

  return 0;
}
