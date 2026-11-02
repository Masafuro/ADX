"""
M0 Bring-up Hardware Test Script for Raspberry Pi Pico H + Waveshare Pico-2CH-RS485

テスト内容:
1. Pico オンボード LED (GP25) の点滅
2. CH0 (GP0/GP1) と CH1 (GP4/GP5) の TX/RX LED 点滅および UART ループバック通信
   (※ A0-A1, B0-B1, GND-GND をジャンパ結線している前提)
"""

import time
from machine import Pin, UART

def run_test():
    print("=== RPR4 Pre-dev M0 Bring-up Test Start ===")

    # 1. Pico オンボード LED (GP25)
    led = Pin(25, Pin.OUT)
    print("Step 1: Pico Onboard LED (GP25) Test...")
    for _ in range(5):
        led.toggle()
        time.sleep(0.1)
    led.value(1) # 点灯維持

    # 2. UART 初期化
    # CH0: GP0 (TX), GP1 (RX)
    # CH1: GP4 (TX), GP5 (RX)
    baud = 115200
    uart0 = UART(0, baudrate=baud, tx=Pin(0), rx=Pin(1), timeout=100)
    uart1 = UART(1, baudrate=baud, tx=Pin(4), rx=Pin(5), timeout=100)

    print(f"Step 2: UART Init (Baudrate: {baud})")
    print("  CH0 -> UART0 (TX: GP0, RX: GP1)")
    print("  CH1 -> UART1 (TX: GP4, RX: GP5)")

    # 3. ループバックテスト (CH0 TX -> CH1 RX)
    test_msg = b"Hello from RPR4 CH0!"
    print(f"\nStep 3: Sending from CH0 to CH1: '{test_msg.decode()}'")
    
    # バッファクリア
    while uart1.any():
        uart1.read()

    uart0.write(test_msg)
    time.sleep(0.05) # 伝送待ち

    rx_data = uart1.read()
    if rx_data:
        print(f"  [PASS] CH1 Received: '{rx_data.decode(errors='replace')}'")
        if rx_data == test_msg:
            print("  ==> CH0 -> CH1 Data Match: OK!")
        else:
            print("  ==> CH0 -> CH1 Data Mismatch!")
    else:
        print("  [WARN] CH1 Received nothing. (Check A0-A1, B0-B1 jumpers)")

    # 4. ループバックテスト (CH1 TX -> CH0 RX)
    test_msg_rev = b"Echo back from RPR4 CH1!"
    print(f"\nStep 4: Sending from CH1 to CH0: '{test_msg_rev.decode()}'")

    while uart0.any():
        uart0.read()

    uart1.write(test_msg_rev)
    time.sleep(0.05)

    rx_data_rev = uart0.read()
    if rx_data_rev:
        print(f"  [PASS] CH0 Received: '{rx_data_rev.decode(errors='replace')}'")
        if rx_data_rev == test_msg_rev:
            print("  ==> CH1 -> CH0 Data Match: OK!")
        else:
            print("  ==> CH1 -> CH0 Data Mismatch!")
    else:
        print("  [WARN] CH0 Received nothing. (Check A0-A1, B0-B1 jumpers)")

    print("\n=== M0 Bring-up Test Complete ===")

if __name__ == "__main__":
    run_test()

