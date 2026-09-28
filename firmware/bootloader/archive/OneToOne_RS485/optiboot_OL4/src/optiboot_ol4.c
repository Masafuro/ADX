/*
 * Optiboot_OL4: 1-to-1 Half-Duplex RS-485 High-Reliability Bootloader (v2.0)
 * Target: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)
 * Protocol: Stop-and-Wait ARQ with 64-byte Flash Page CRC-16 Verification
 *
 * Hardware Pin Mapping:
 *   PA1: USART0 TXD (Alternate Pin via PORTMUX)
 *   PA2: USART0 RXD (Alternate Pin)
 *   PA4: RS-485 DE  (Driver Enable, Active HIGH via VPORTA)
 *   PA7: RS-485 /RE (Receiver Enable, Active LOW via VPORTA)
 *   PB2: Onboard RED LED (Status indicator, Active HIGH via VPORTB)
 *
 * Memory Layout:
 *   FUSE.BOOTEND = 0x04 -> 1024 Bytes (0x0000 - 0x03FF) [Hardware Protected]
 *   Application Start = 0x0400 (Page 0x10, 15 KB available)
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay_basic.h>
#include "ln485_protocol.h"

unsigned const int __attribute__((section(".version"))) __attribute__((used))
optiboot_version = 256 * OL4_MAJVER + OL4_MINVER;

#define BAUD_RATE 9600L
#define BAUD_SETTING_16 (((16000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1111 (0x0457)
#define BAUD_SETTING_20 (((20000000UL / 6) * 64) / (16L * BAUD_RATE)) // 1389 (0x056D)

// Static buffer for 64-byte Flash page (allocated in SRAM, avoiding naked stack issues)
static uint8_t page_buf[FLASH_PAGE_SIZE];

// =========================================================================
// CRC-16-CCITT (Poly: 0x1021, Init: 0xFFFF)
// =========================================================================
static uint16_t crc16_update(uint16_t crc, uint8_t data) {
  crc ^= ((uint16_t)data << 8);
  for (uint8_t i = 0; i < 8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}

// =========================================================================
// RS-485 Transceiver & UART Direction Control (Golden Ratio Timings)
// =========================================================================
static inline void rs485_set_tx(void) {
  VPORTA.OUT |= (1 << 4) | (1 << 7); // DE=1, /RE=1 (Transmit Mode, Mute Receiver)
  _delay_loop_2(830);                // ~1ms transceiver setup delay
}

static inline void rs485_set_rx(void) {
  // Wait until last bit is completely out of the transmitter
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm;     // Clear flag
  _delay_loop_2(830);                // ~1ms bus release settle delay
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0 (Receive Mode)
}

static inline void putch(uint8_t ch) {
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  USART0.TXDATAL = ch;
}

// Sends an 8-byte fixed-frame response packet
static void send_response(uint8_t seq, uint8_t resp, uint8_t page_no, uint8_t status, uint8_t extra) {
  uint8_t chk = (PKT_STX + seq + resp + page_no + status + extra) & 0xFF;

  rs485_set_tx();
  putch(PKT_STX);
  putch(seq);
  putch(resp);
  putch(page_no);
  putch(status);
  putch(extra);
  putch(chk);
  putch(PKT_ETX);
  rs485_set_rx();
}

// Reads a byte with timeout (~40ms at 3.33MHz).
// Returns 0x00-0xFF on success, or 0xFFFF on timeout.
static uint16_t getch_timeout(void) {
  uint16_t timeout = 0;
  while (!(USART0.STATUS & USART_RXCIF_bm)) {
    if (++timeout > 16000) {
      return 0xFFFF; // Timeout
    }
  }
  (void)USART0.RXDATAH;
  return USART0.RXDATAL;
}

// Jump to User Application at 0x0400
static void app_start(void) __attribute__((noreturn));
static void app_start(void) {
  // Disable USART0 and restore default state
  USART0.CTRLB = 0;
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // DE=0, /RE=0
  VPORTB.OUT &= ~(1 << 2);              // LED OFF

  // Jump to start of application section (0x0400)
  __asm__ __volatile__ (
    "jmp 0x0400\n\t"
  );
  while (1);
}

// =========================================================================
// Main Entry Point
// =========================================================================
int main(void) __attribute__((naked)) __attribute__((section(".init9")));
int main(void) {
  // 1. Initialize Stack Pointer and clear r1 (__zero_reg__)
  __asm__ __volatile__("clr r1\n\t");
  SPL = 0xFF;
  SPH = 0x3F;

  // 2. Hardware Pin Setup
  //    PA1(TX)=OUT, PA4(DE)=OUT, PA7(/RE)=OUT, PB2(LED)=OUT
  VPORTA.DIR |= (1 << 1) | (1 << 4) | (1 << 7);
  VPORTA.OUT |= (1 << 1);               // PA1=HIGH
  VPORTA.OUT &= ~((1 << 4) | (1 << 7)); // Start in RX Mode

  VPORTB.DIR |= (1 << 2);
  VPORTB.OUT &= ~(1 << 2);              // LED OFF

  // Route USART0 to alternate pins PA1/PA2
  PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

  // 3. USART0 9,600 bps Configuration (Runtime clock detection via FUSE_OSCCFG)
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

  // 4. Power-on Entry Timeout (~1.0 second wait for CMD_PING)
  // If no STX is received within ~1.0 second, directly boot user app!
  uint8_t bootloader_active = 0;
  uint16_t entry_loop = 0;
  uint8_t entry_seconds = 0;

  while (!bootloader_active) {
    if (USART0.STATUS & USART_RXCIF_bm) {
      (void)USART0.RXDATAH;
      uint8_t b = USART0.RXDATAL;
      if (b == PKT_STX) {
        bootloader_active = 1;
        VPORTB.OUT |= (1 << 2); // Turn LED ON (Bootloader locked)
        break;
      }
    }

    if (++entry_loop == 0) {
      VPORTB.IN |= (1 << 2); // Toggle LED while waiting (~3Hz)
      if (++entry_seconds >= 6) { // ~1.0s elapsed
        app_start();
      }
    }
  }

  // 5. Main Bootloader Command Loop (Stop-and-Wait ARQ)
  for (;;) {
    uint16_t rx_val;

    // Wait for STX
    rx_val = getch_timeout();
    if (rx_val != PKT_STX)
      continue;

    // Receive Header: SEQ, CMD, PAGE_NO, LEN
    uint16_t r_seq = getch_timeout();
    if (r_seq > 0xFF) continue;
    uint16_t r_cmd = getch_timeout();
    if (r_cmd > 0xFF) continue;
    uint16_t r_page = getch_timeout();
    if (r_page > 0xFF) continue;
    uint16_t r_len = getch_timeout();
    if (r_len > 0xFF) continue;

    uint8_t seq = (uint8_t)r_seq;
    uint8_t cmd = (uint8_t)r_cmd;
    uint8_t page_no = (uint8_t)r_page;
    uint8_t len = (uint8_t)r_len;

    uint16_t calc_crc = 0xFFFF;
    calc_crc = crc16_update(calc_crc, PKT_STX);
    calc_crc = crc16_update(calc_crc, seq);
    calc_crc = crc16_update(calc_crc, cmd);
    calc_crc = crc16_update(calc_crc, page_no);
    calc_crc = crc16_update(calc_crc, len);

    // Receive Payload (if any, up to 64 bytes)
    uint8_t rx_failed = 0;
    for (uint8_t i = 0; i < len; i++) {
      uint16_t d = getch_timeout();
      if (d > 0xFF) {
        rx_failed = 1;
        break;
      }
      if (i < FLASH_PAGE_SIZE) {
        page_buf[i] = (uint8_t)d;
      }
      calc_crc = crc16_update(calc_crc, (uint8_t)d);
    }
    if (rx_failed) continue;

    // Receive CRC16 (2 bytes: H, L) and ETX
    uint16_t r_crch = getch_timeout();
    if (r_crch > 0xFF) continue;
    uint16_t r_crcl = getch_timeout();
    if (r_crcl > 0xFF) continue;
    uint16_t r_etx = getch_timeout();
    if (r_etx != PKT_ETX) continue;

    uint16_t rx_crc = ((uint16_t)r_crch << 8) | (uint8_t)r_crcl;

    // Verify CRC-16
    if (calc_crc != rx_crc) {
      // Reject and request retransmission (ARQ)
      send_response(seq, RESP_NAK, page_no, STATUS_ERR_CRC, 0);
      continue;
    }

    // Process Valid Command
    if (cmd == CMD_PING) {
      send_response(seq, RESP_ACK, page_no, STATUS_OK, (OL4_MAJVER << 4) | OL4_MINVER);

    } else if (cmd == CMD_GET_CHIP_INFO) {
      // Return ATtiny1616 signature byte 2 (0x21)
      send_response(seq, RESP_ACK, page_no, STATUS_OK, CHIP_SIG2);

    } else if (cmd == CMD_WRITE_PAGE) {
      // BOOTEND Safety Guard: Page < 0x10 (0x0400) is strictly protected!
      if (page_no < APP_START_PAGE || page_no >= FLASH_TOTAL_PAGES || len != FLASH_PAGE_SIZE) {
        send_response(seq, RESP_NAK, page_no, STATUS_ERR_PROTECTED, 0);
      } else {
        // Load Page Buffer into Unified Flash Memory (0x8000 + page_offset)
        uint8_t *flash_ptr = (uint8_t *)(MAPPED_PROGMEM_START + ((uint16_t)page_no * FLASH_PAGE_SIZE));
        for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
          *(flash_ptr++) = page_buf[i];
        }

        // Commit Page Erase & Write (NVMCTRL)
        _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
        while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm))
          ;

        send_response(seq, RESP_ACK, page_no, STATUS_OK, 0);
      }

    } else if (cmd == CMD_VERIFY_PAGE) {
      if (page_no >= FLASH_TOTAL_PAGES) {
        send_response(seq, RESP_NAK, page_no, STATUS_ERR_PROTECTED, 0);
      } else {
        // Calculate CRC16 of the Flash Page directly
        uint16_t v_crc = 0xFFFF;
        const uint8_t *flash_ptr = (const uint8_t *)(MAPPED_PROGMEM_START + ((uint16_t)page_no * FLASH_PAGE_SIZE));
        for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
          v_crc = crc16_update(v_crc, *(flash_ptr++));
        }
        // Return lower byte of CRC as EXTRA, ACK
        send_response(seq, RESP_ACK, page_no, STATUS_OK, (uint8_t)(v_crc & 0xFF));
      }

    } else if (cmd == CMD_BOOT_APP) {
      send_response(seq, RESP_ACK, page_no, STATUS_OK, 0);
      _delay_loop_2(8300); // ~10ms settle delay
      app_start();

    } else {
      send_response(seq, RESP_NAK, page_no, STATUS_ERR_UNKNOWN, 0);
    }
  }

  return 0;
}
