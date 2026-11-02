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

// Flush Pulse Measurement FIFO
void flush_pulse_measure(void) {
    while (!pio_sm_is_rx_fifo_empty(pio0, measure_sm)) {
        (void)pio_sm_get(pio0, measure_sm);
    }
}

// Get measured pulse width in microseconds (timeout: 50ms)
bool get_pulse_width_us(float *width_us) {
    absolute_time_t deadline = make_timeout_time_ms(50);
    while (pio_sm_is_rx_fifo_empty(pio0, measure_sm)) {
        if (time_reached(deadline)) return false;
        sleep_us(10);
    }
    uint32_t count = pio_sm_get_blocking(pio0, measure_sm);
    // 2MHz clock, 2 cycles per loop = 1.0us per count
    *width_us = (float)count * 1.0f;
    return true;
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
    uart_get_hw(uart1)->rsr = 0;
}

// Drain all pending bytes from UART1 (timeout: 20ms)
size_t uart1_drain_all(uint8_t *buf, size_t max_buf) {
    size_t count = 0;
    absolute_time_t deadline = make_timeout_time_ms(20);

    while (count < max_buf && !time_reached(deadline)) {
        if (uart_is_readable(uart1)) {
            buf[count++] = (uint8_t)uart_getc(uart1);
            deadline = make_timeout_time_ms(10);
        } else {
            sleep_us(10);
        }
    }
    return count;
}

// ============================================================================
// Test 1: PIO LIN BREAK Pulse Width Verification
// ============================================================================
bool run_test_1(void) {
    printf("\n======================================================\n");
    printf("   [Test 1] PIO LIN BREAK Pulse Width Verification\n");
    printf("======================================================\n");

    const float expected_us = (13.0f / (float)TEST_BAUD) * 1000000.0f; // 112.85 us
    const float tolerance = 0.05f; // ±5%
    const float min_us = expected_us * (1.0f - tolerance);
    const float max_us = expected_us * (1.0f + tolerance);

    printf("Expected: %.2f us (Tolerance: +/-5.0%% => %.2f ~ %.2f us)\n",
           expected_us, min_us, max_us);

    flush_pulse_measure();
    flush_uart1();

    // Send single LIN BREAK
    ln485_tx_send_break(&ch0_tx);
    ln485_tx_wait_idle(&ch0_tx);

    float measured_us = 0.0f;
    if (!get_pulse_width_us(&measured_us)) {
        printf("[FAIL] Timeout waiting for pulse measurement!\n");
        return false;
    }

    float error_pct = ((measured_us - expected_us) / expected_us) * 100.0f;
    bool pass = (measured_us >= min_us && measured_us <= max_us);

    printf("Measured: %.2f us (Error: %+.2f%%)\n", measured_us, error_pct);
    printf("Result  : %s\n", pass ? "PASS" : "FAIL");

    return pass;
}

// ============================================================================
// Test 2: Single 8-Byte LN-485 Frame Verification
// ============================================================================
bool run_test_2(void) {
    printf("\n======================================================\n");
    printf("   [Test 2] Single 8-Byte Frame Loopback Verification\n");
    printf("======================================================\n");

    const uint8_t test_pid = 0x20;
    const uint8_t test_payload[LN485_PAYLOAD_SIZE] = {'R', 'P', 'R', '4', 'L', 'N', '4', '8'};
    const uint8_t expected_chk = ln485_calc_checksum(test_pid, test_payload, LN485_PAYLOAD_SIZE, true);

    printf("1. TX Frame Structure (LIN/CAN standard: 8-byte payload):\n");
    printf("   - BREAK:    13 bits Low (≒ 112.85 us)\n");
    printf("   - SYNC:     0x%02X\n", LN485_SYNC_BYTE);
    printf("   - PID:      0x%02X\n", test_pid);
    printf("   - Payload:  \"%.*s\" (8 bytes)\n", LN485_PAYLOAD_SIZE, test_payload);
    printf("   - Checksum: 0x%02X (Enhanced)\n", expected_chk);
    printf("   ==> Total Frame Size: 11 bytes (+ BREAK marker => 12 bytes at RX)\n\n");

    flush_uart1();

    // Transmit standard 8-byte frame
    ln485_tx_send_standard_frame(&ch0_tx, test_pid, test_payload);
    ln485_tx_wait_idle(&ch0_tx);

    // Read hardware status register
    uint32_t rsr = uart_get_hw(uart1)->rsr;
    bool oe = (rsr & UART_UARTRSR_OE_BITS) != 0;

    // Drain received bytes
    uint8_t rx_buf[64] = {0};
    size_t rx_len = uart1_drain_all(rx_buf, sizeof(rx_buf));

    printf("2. Reception & Verification:\n");
    printf("   - Received Bytes: %zu bytes (Expected: 12 bytes)\n", rx_len);
    printf("   - Hardware FIFO Overrun (OE): %s\n", oe ? "YES (FAIL)" : "NO (OK)");

    printf("   - Raw Hex Dump:\n     ");
    for (size_t i = 0; i < rx_len; i++) {
        printf("%02X ", rx_buf[i]);
    }
    printf("\n");

    // Field-by-field verification
    bool ok_len = (rx_len == 12);
    bool ok_break = (rx_len >= 1 && rx_buf[0] == 0x00);
    bool ok_sync = (rx_len >= 2 && rx_buf[1] == LN485_SYNC_BYTE);
    bool ok_pid = (rx_len >= 3 && rx_buf[2] == test_pid);
    bool ok_data = (rx_len >= 11 && memcmp(&rx_buf[3], test_payload, LN485_PAYLOAD_SIZE) == 0);
    bool ok_chk = (rx_len >= 12 && rx_buf[11] == expected_chk);
    bool ok_oe = !oe;

    printf("   - Field Checks:\n");
    printf("     [BREAK == 0x00]     : %s\n", ok_break ? "OK" : "NG");
    printf("     [SYNC  == 0x55]     : %s\n", ok_sync ? "OK" : "NG");
    printf("     [PID   == 0x%02X]     : %s\n", test_pid, ok_pid ? "OK" : "NG");
    printf("     [Payload (8B) Match]: %s\n", ok_data ? "OK" : "NG");
    printf("     [Checksum Match]    : %s\n", ok_chk ? "OK" : "NG");
    printf("     [No Overrun Error]  : %s\n", ok_oe ? "OK" : "NG");

    bool pass = ok_len && ok_break && ok_sync && ok_pid && ok_data && ok_chk && ok_oe;
    printf("Result: %s\n", pass ? "PASS" : "FAIL");

    return pass;
}

// ============================================================================
// Test 3: 1,000 Continuous Packet Loopback Stress Test
// ============================================================================
bool run_test_3(void) {
    const int total_packets = 1000;
    printf("\n======================================================\n");
    printf("   [Test 3] %d Continuous 8-Byte Packet Stress Test\n", total_packets);
    printf("======================================================\n");
    printf(" Transmitting %d consecutive LN-485 frames at 115200 bps...\n", total_packets);

    int success_count = 0;
    int overrun_count = 0;
    int mismatch_count = 0;

    for (int i = 0; i < total_packets; i++) {
        uint8_t pid = (uint8_t)(0x10 + (i % 48));
        uint8_t payload[LN485_PAYLOAD_SIZE];
        for (int b = 0; b < LN485_PAYLOAD_SIZE; b++) {
            payload[b] = (uint8_t)((i + b * 17) & 0xFF);
        }
        uint8_t expected_chk = ln485_calc_checksum(pid, payload, LN485_PAYLOAD_SIZE, true);

        flush_uart1();

        // Send frame
        ln485_tx_send_standard_frame(&ch0_tx, pid, payload);
        ln485_tx_wait_idle(&ch0_tx);

        // Check hardware error
        uint32_t rsr = uart_get_hw(uart1)->rsr;
        if (rsr & UART_UARTRSR_OE_BITS) {
            overrun_count++;
        }

        // Receive
        uint8_t rx_buf[32] = {0};
        size_t rx_len = uart1_drain_all(rx_buf, sizeof(rx_buf));

        // Verify
        if (rx_len == 12 &&
            rx_buf[0] == 0x00 &&
            rx_buf[1] == LN485_SYNC_BYTE &&
            rx_buf[2] == pid &&
            memcmp(&rx_buf[3], payload, LN485_PAYLOAD_SIZE) == 0 &&
            rx_buf[11] == expected_chk &&
            !(rsr & UART_UARTRSR_OE_BITS)) {
            success_count++;
        } else {
            mismatch_count++;
        }

        // Progress display every 100 packets
        if ((i + 1) % 100 == 0) {
            printf("   Progress: %4d / %4d packets | Success: %4d | Errors: %d\n",
                   i + 1, total_packets, success_count, mismatch_count);
        }

        // Small interval between frames (1ms)
        sleep_ms(1);
    }

    float loss_rate = ((float)mismatch_count / (float)total_packets) * 100.0f;

    printf("\n--- Test 3 Summary ---\n");
    printf("Total Packets Transmitted : %d\n", total_packets);
    printf("Successful Packets        : %d\n", success_count);
    printf("Failed / Corrupted Packets: %d\n", mismatch_count);
    printf("Overrun (OE) Errors       : %d\n", overrun_count);
    printf("Packet Loss Rate          : %.2f%%\n", loss_rate);

    bool pass = (success_count == total_packets && mismatch_count == 0);
    printf("Result: %s (Criterion: 0 errors / 0.00%% loss)\n", pass ? "PASS" : "FAIL");

    return pass;
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
    printf("   ADX RPR4 M2 Verification: PIO LN-485 (8-Byte Frame)\n");
    printf("======================================================\n");
    printf("Baud Rate: %d bps\n", TEST_BAUD);
    printf("Protocol : LN-485 8-Byte Standard Frame (LIN 2.x & CAN Compatible)\n");
    printf("Frame    : BREAK + SYNC(0x55) + PID + 8B DATA + CHECKSUM = 12B Total\n");

    // Init CH0 TX (PIO)
    if (!ln485_tx_init(&ch0_tx, pio0, 0, PIN_CH0_TX, TEST_BAUD)) {
        printf("[FATAL] Failed to initialize PIO TX!\n");
        while (1) tight_loop_contents();
    }

    // Init Pulse Measurement (PIO0 SM1) on GP5
    init_pulse_measure();

    // Init CH1 RX (UART1) on GP5
    init_uart1_rx(TEST_BAUD);

    sleep_ms(500);

    // Run Test 1
    bool t1 = run_test_1();
    sleep_ms(300);

    // Run Test 2
    bool t2 = run_test_2();
    sleep_ms(300);

    // Run Test 3
    bool t3 = run_test_3();

    printf("\n======================================================\n");
    printf("   M2 Overall Verification Result: %s\n",
           (t1 && t2 && t3) ? "ALL PASS (COMPLETE)" : "FAIL");
    printf("======================================================\n\n");

    // Indicator (Blink fast if all pass, blink slow if fail)
    uint32_t delay_ms = (t1 && t2 && t3) ? 150 : 800;
    while (1) {
        gpio_xor_mask(1ul << PIN_LED);
        sleep_ms(delay_ms);
    }

    return 0;
}
