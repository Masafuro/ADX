/*
 * Robust RS-485 Packet Benchmark Firmware @ 9,600 bps (Step 2)
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
 *     PB2: Onboard RED LED (Blinks on RS-485 packet activity)
 *
 * Packet Format (8 Bytes Fixed Frame):
 *   +------+------+------+-------+-------+-------+----------+------+
 *   | STX  | SEQ  | CMD  | DATA0 | DATA1 | DATA2 | CHECKSUM | ETX  |
 *   | 0x02 | 0-FF | code | 8-bit | 8-bit | 8-bit |  8-bit   | 0x03 |
 *   +------+------+------+-------+-------+-------+----------+------+
 *   CHECKSUM = (STX + SEQ + CMD + DATA0 + DATA1 + DATA2) & 0xFF
 */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay_basic.h>

#define BAUD_RATE 9600L
#define BAUD_SETTING_16 (((16000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1111 (0x0457)
#define BAUD_SETTING_20 (((20000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1389 (0x056D)

#define PKT_STX         0x02
#define PKT_ETX         0x03
#define PKT_LEN         8

// Commands
#define CMD_PING        0x01
#define CMD_ECHO        0x02
#define CMD_GET_STATS   0x03

// Responses
#define RESP_ACK        0x06
#define RESP_NAK_CHKSUM 0x15
#define RESP_NAK_CMD    0x16

static uint16_t total_packets = 0;
static uint16_t checksum_errors = 0;
static uint16_t framing_errors = 0;

// =========================================================================
// Channel 2: CH342K Independent Soft-UART TX on PB4 @ 9,600 bps
// =========================================================================
static void dbg_putch(uint8_t c) {
  // Start bit: LOW
  VPORTB.OUT &= ~(1 << 4);
  _delay_loop_2(86); // ~103.2 us @ 3.33MHz

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
// Channel 1: RS-485 Hardware Transceiver & UART Operations
// =========================================================================
static inline void set_tx_mode(void) {
  VPORTA.OUT |= (1 << 4) | (1 << 7); // DE=1, /RE=1
  _delay_loop_2(830);                // ~1ms transceiver setup delay
}

static inline void set_rx_mode(void) {
  // Wait until last bit is completely out
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm;     // Clear flag
  _delay_loop_2(830);                // ~1ms bus release settle delay
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0
}

static inline void rs485_putch(uint8_t ch) {
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  USART0.TXDATAL = ch;
}

static void send_response_packet(uint8_t seq, uint8_t resp_code, uint8_t d0, uint8_t d1, uint8_t d2) {
  uint8_t chk = PKT_STX + seq + resp_code + d0 + d1 + d2;

  set_tx_mode();
  rs485_putch(PKT_STX);
  rs485_putch(seq);
  rs485_putch(resp_code);
  rs485_putch(d0);
  rs485_putch(d1);
  rs485_putch(d2);
  rs485_putch(chk);
  rs485_putch(PKT_ETX);
  set_rx_mode();
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
  VPORTA.DIR |= (1 << 1) | (1 << 4) | (1 << 7);
  VPORTA.OUT |= (1 << 1);               // PA1=HIGH
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0 (RX Mode)

  VPORTB.DIR |= (1 << 2) | (1 << 4);
  VPORTB.OUT &= ~(1 << 2);              // Red LED OFF
  VPORTB.OUT |= (1 << 4);               // PB4 (STX) = HIGH

  // Route USART0 to alternate pins PA1(TXD) / PA2(RXD)
  PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

  // 9,600 bps Normal Asynchronous UART
  if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
    USART0.BAUD = BAUD_SETTING_16;
  } else {
    USART0.BAUD = BAUD_SETTING_20;
  }
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLA = 0;
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;

  // Flush startup buffer
  while (USART0.STATUS & USART_RXCIF_bm) {
    (void)USART0.RXDATAH;
    (void)USART0.RXDATAL;
  }

  // Initial Announcement on COM21
  dbg_print("\r\n=======================================================\r\n");
  dbg_print(" [BOOT] Core-D RS-485 Packet Benchmark v1.0 @ 9600 bps\r\n");
  dbg_print(" Protocol: 8-Byte Fixed Frame (STX=0x02, ETX=0x03)\r\n");
  dbg_print(" USART0: Normal UART BAUD=0x");
  dbg_print_hex16(USART0.BAUD);
  dbg_print("\r\n DE=PA4, /RE=PA7 (Initialized to RX Mode)\r\n");
  dbg_print("=======================================================\r\n");

  static uint8_t rx_buf[PKT_LEN];
  static uint8_t rx_idx = 0;
  static uint16_t byte_timeout = 0;
  static uint16_t hb_counter = 0;

  for (;;) {
    // 1. Check if byte received on RS-485
    if (USART0.STATUS & USART_RXCIF_bm) {
      (void)USART0.RXDATAH;
      uint8_t b = USART0.RXDATAL;

      if (rx_idx == 0) {
        // Waiting for STX (0x02)
        if (b == PKT_STX) {
          rx_buf[0] = b;
          rx_idx = 1;
          byte_timeout = 0;
          VPORTB.OUT |= (1 << 2); // Turn on Red LED
        }
      } else {
        // Collect remaining 7 bytes
        rx_buf[rx_idx++] = b;
        byte_timeout = 0;

        if (rx_idx == PKT_LEN) {
          // Packet complete!
          total_packets++;

          uint8_t seq  = rx_buf[1];
          uint8_t cmd  = rx_buf[2];
          uint8_t d0   = rx_buf[3];
          uint8_t d1   = rx_buf[4];
          uint8_t d2   = rx_buf[5];
          uint8_t chk  = rx_buf[6];
          uint8_t etx  = rx_buf[7];

          // Compute expected checksum
          uint8_t expected_chk = PKT_STX + seq + cmd + d0 + d1 + d2;

          if (etx != PKT_ETX) {
            framing_errors++;
            dbg_print("[PKT ERR #");
            dbg_print_hex16(total_packets);
            dbg_print("] Bad ETX: 0x");
            dbg_print_hex8(etx);
            dbg_print("\r\n");
          } else if (chk != expected_chk) {
            checksum_errors++;
            // Send NAK immediately
            send_response_packet(seq, RESP_NAK_CHKSUM, expected_chk, chk, 0);
            dbg_print("[PKT ERR #");
            dbg_print_hex16(total_packets);
            dbg_print("] Bad Chksum: Got 0x");
            dbg_print_hex8(chk);
            dbg_print(" Exp 0x");
            dbg_print_hex8(expected_chk);
            dbg_print("\r\n");
          } else {
            // Valid Packet! Send response immediately for lowest latency & jitter
            if (cmd == CMD_PING) {
              send_response_packet(seq, RESP_ACK, d0, 0xAA, (uint8_t)(total_packets & 0xFF));
            } else if (cmd == CMD_ECHO) {
              send_response_packet(seq, RESP_ACK, d0, d1, d2);
            } else if (cmd == CMD_GET_STATS) {
              send_response_packet(seq, RESP_ACK, (uint8_t)(total_packets >> 8), (uint8_t)(total_packets & 0xFF), (uint8_t)(checksum_errors & 0xFF));
            } else {
              send_response_packet(seq, RESP_NAK_CMD, cmd, 0, 0);
            }

            // Compact Telemetry on COM21 (18 chars ~ 18ms)
            dbg_print("[PKT #");
            dbg_print_hex16(total_packets);
            dbg_print("] SEQ=0x");
            dbg_print_hex8(seq);
            dbg_print(" OK\r\n");
          }

          // Reset receiver for next packet
          rx_idx = 0;
          hb_counter = 0;          // Suppress IDLE heartbeat during active traffic
          VPORTB.OUT &= ~(1 << 2); // Turn off Red LED
        }
      }
    }

    // 2. Inter-byte Timeout Check (reset if next byte takes > ~30ms)
    if (rx_idx > 0) {
      if (++byte_timeout > 12000) { // ~30ms at 3.33MHz
        dbg_print("[PKT TO] Incomplete pkt discarded (bytes=");
        dbg_putch('0' + rx_idx);
        dbg_print(")\r\n");
        rx_idx = 0;
        byte_timeout = 0;
        VPORTB.OUT &= ~(1 << 2);
      }
    }

    // 3. Periodic Background Telemetry (~1Hz)
    if (++hb_counter == 0) {
      dbg_print("[IDLE 9600] TotalPkt=0x");
      dbg_print_hex16(total_packets);
      dbg_print(" ChkErr=0x");
      dbg_print_hex16(checksum_errors);
      dbg_print(" PA2=");
      dbg_putch((VPORTA.IN & (1 << 2)) ? '1' : '0');
      dbg_print("\r\n");
    }
  }

  return 0;
}
