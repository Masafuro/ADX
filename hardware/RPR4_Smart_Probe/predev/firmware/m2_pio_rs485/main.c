#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/structs/uart.h"
#include "hardware/regs/uart.h"
#include "ln485_driver.h"
#include "ln485.pio.h"

// Pin definitions (Waveshare Pico-2CH-RS485)
#define PIN_CH0_TX          0   // PIO TX
#define PIN_CH0_RX          1
#define PIN_CH1_TX          4
#define PIN_CH1_RX          5   // UART1 RX & Pulse Measure
#define PIN_LED             25

#define TEST_BAUD           115200

static ln485_tx_t ch0_tx;
static uint measure_sm = 1;
static uint measure_offset;

// Pulse measurement PIO init
void init_pulse_measure(void) {
    measure_offset = pio_add_program(pio0, &ln485_pulse_measure_program);
    ln485_pulse_measure_program_init(pio0, measure_sm, measure_offset, PIN_CH1_RX);
}

// UART1 (CH1 RX) init
void init_uart1_rx(uint baud) {
    uart_init(uart1, baud);
    gpio_set_function(PIN_CH1_RX, GPIO_FUNC_UART);
    gpio_set_input_enabled(PIN_CH1_RX, true);
    uart_set_hw_flow(uart1, false, false);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart1, true);
}

// Flush UART1 RX and clear error register
void flush_uart1(void) {
    while (uart_is_readable(uart1)) {
        (void)uart_getc(uart1);
    }
    // Clear UARTRSR error register (write to clear)
    uart_get_hw(uart1)->rsr = 0;
}

// Drain all pending bytes from UART1 (timeout: 30ms)
size_t uart1_drain_all(uint8_t *buf, size_t max_buf) {
    size_t count = 0;
    absolute_time_t deadline = make_timeout_time_ms(30);

    while (count < max_buf && !time_reached(deadline)) {
        if (uart_is_readable(uart1)) {
            buf[count++] = (uint8_t)uart_getc(uart1);
            deadline = make_timeout_time_ms(15);
        } else {
            sleep_us(10);
        }
    }
    return count;
}

// ============================================================================
// Investigation 1 & 2: Hardware Status Register & Raw Byte Count
// ============================================================================
void run_investigation_1_and_2(void) {
    printf("\n======================================================\n");
    printf("   [INV 1 & 2] Test 2 Packet Hardware State & Byte Count\n");
    printf("======================================================\n");

    const uint8_t test_payload[] = "RPR4-PIO-RS485-LN485-TEST-2026";
    size_t payload_len = sizeof(test_payload); // 31 bytes

    flush_uart1();

    printf("1. TX Packet Structure:\n");
    printf("   - LIN BREAK (13 bits Low ≒ 112us)\n");
    printf("   - SYNC Byte: 0x55 (1 byte)\n");
    printf("   - Payload:   \"%s\" (%zu bytes)\n", test_payload, payload_len);
    printf("   ==> Total Bytes Sent: 1(SYNC) + %zu(Payload) = %zu bytes (+ BREAK)\n\n",
           payload_len, 1 + payload_len);

    // Transmit
    ln485_tx_send_frame(&ch0_tx, test_payload, payload_len, true);
    ln485_tx_wait_idle(&ch0_tx);

    // [INV 1] Read UART1 Hardware Status Register (UARTRSR) immediately
    uart_hw_t *hw = uart_get_hw(uart1);
    uint32_t rsr = hw->rsr;
    bool oe = (rsr & UART_UARTRSR_OE_BITS) != 0; // Overrun Error (FIFO overflow)
    bool be = (rsr & UART_UARTRSR_BE_BITS) != 0; // Break Error (BREAK detected)
    bool fe = (rsr & UART_UARTRSR_FE_BITS) != 0; // Framing Error
    bool pe = (rsr & UART_UARTRSR_PE_BITS) != 0; // Parity Error

    printf("2. Hardware Status Register (UARTRSR):\n");
    printf("   - Raw Value (Hex): 0x%02X\n", (unsigned int)rsr);
    printf("   - OE (Overrun Error / FIFO Overflow) : %s\n", oe ? "YES (OVERFLOW DETECTED!)" : "NO");
    printf("   - BE (Break Error / BREAK Signal)    : %s\n", be ? "YES (BREAK DETECTED!)" : "NO");
    printf("   - FE (Framing Error)                 : %s\n", fe ? "YES" : "NO");
    printf("   - PE (Parity Error)                  : %s\n\n", pe ? "YES" : "NO");

    // [INV 2] Drain all bytes remaining in FIFO
    uint8_t rx_raw[128] = {0};
    size_t rx_count = uart1_drain_all(rx_raw, sizeof(rx_raw));

    printf("3. Actual Received Bytes & Raw Dump:\n");
    printf("   - Bytes received in RX FIFO: %zu bytes (Expected: 32 or 33 bytes)\n", rx_count);
    printf("   - Hex Dump:\n     ");
    for (size_t i = 0; i < rx_count; i++) {
        printf("%02X ", rx_raw[i]);
        if ((i + 1) % 16 == 0) printf("\n     ");
    }
    printf("\n");

    printf("   - ASCII Dump:\n     \"");
    for (size_t i = 0; i < rx_count; i++) {
        char c = (char)rx_raw[i];
        if (c >= 32 && c <= 126) putchar(c);
        else printf("[%02X]", (uint8_t)c);
    }
    printf("\"\n\n");

    if (oe && rx_count == 32) {
        printf("==> [CONCLUSION for INV 1 & 2]\n");
        printf("    OE = YES and rx_count = 32 confirmed!\n");
        printf("    Hypothesis verified: Hardware FIFO stopped at exactly 32 bytes.\n");
    }
}

// ============================================================================
// Investigation 3: Packet Size Sweep Experiment (8B to 64B)
// ============================================================================
void run_investigation_3_sweep(void) {
    printf("\n======================================================\n");
    printf("   [INV 3] Packet Size Sweep Experiment (FIFO Boundary)\n");
    printf("======================================================\n");
    printf(" Transmitting various payload sizes while waiting for TX completion.\n");
    printf(" Observing where FIFO overflow (OE) begins.\n\n");

    const size_t test_sizes[] = { 8, 16, 24, 28, 30, 31, 32, 40, 48, 64 };
    const size_t num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    printf("| Payload | Sent Total | Received | Overrun (OE) | Result Match |\n");
    printf("| :---:   | :---:      | :---:    | :---:        | :---:        |\n");

    for (size_t t = 0; t < num_tests; t++) {
        size_t plen = test_sizes[t];
        uint8_t tx_buf[64];
        for (size_t i = 0; i < plen; i++) {
            tx_buf[i] = (uint8_t)('A' + (i % 26));
        }

        flush_uart1();

        // Transmit
        ln485_tx_send_frame(&ch0_tx, tx_buf, plen, true);
        ln485_tx_wait_idle(&ch0_tx);

        // Check register
        uint32_t rsr = uart_get_hw(uart1)->rsr;
        bool oe = (rsr & UART_UARTRSR_OE_BITS) != 0;

        // Drain RX
        uint8_t rx_buf[128] = {0};
        size_t rx_len = uart1_drain_all(rx_buf, sizeof(rx_buf));

        // Verify match (0x00, 0x55, payload)
        bool match = false;
        size_t expected_total = 1 + 1 + plen; // BREAK(1) + SYNC(1) + Payload
        if (rx_len == expected_total && rx_buf[0] == 0x00 && rx_buf[1] == LN485_SYNC_BYTE &&
            memcmp(&rx_buf[2], tx_buf, plen) == 0) {
            match = true;
        }

        printf("| %3zu B   | %3zu B      | %3zu B   | %-12s | %s |\n",
               plen,
               1 + plen,
               rx_len,
               oe ? "OE=1 (OVERRUN)" : "OE=0 (OK)",
               match ? "MATCH (PASS)" : "MISMATCH (FAIL)");

        sleep_ms(20);
    }

    printf("\n======================================================\n");
    printf("   Investigation Complete\n");
    printf("======================================================\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    stdio_init_all();

    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 1);

    // Wait for USB CDC connection (up to 3 seconds)
    for (int i = 0; i < 30; i++) {
        if (stdio_usb_connected()) break;
        sleep_ms(100);
    }

    printf("\n\n======================================================\n");
    printf("   ADX RPR4 M2 Diagnostic & Hypothesis Verification\n");
    printf("======================================================\n");

    // Init CH0 TX (PIO)
    if (!ln485_tx_init(&ch0_tx, pio0, 0, PIN_CH0_TX, TEST_BAUD)) {
        printf("[FATAL] Failed to initialize PIO TX!\n");
        while (1) tight_loop_contents();
    }

    // Init CH1 RX (UART1)
    init_uart1_rx(TEST_BAUD);

    sleep_ms(500);

    // Run Investigation 1 & 2
    run_investigation_1_and_2();
    sleep_ms(200);

    // Run Investigation 3 (Sweep)
    run_investigation_3_sweep();

    // Indicator (Blink)
    while (1) {
        gpio_xor_mask(1ul << PIN_LED);
        sleep_ms(500);
    }

    return 0;
}
