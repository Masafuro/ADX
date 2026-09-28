```powershell

PS C:\Users\User> python debug_flasher.py --port COM19 --read-only --start 0x0200 --pages 11
[INIT] Opening serial port COM19 at 115200 bps...
[INIT] Connected successfully to COM19.

[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 5.66s (probe #47)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 1.9ms) PASS
  [TX] 75 20                    -> [RX] 14 1E 94 21 10               (RTT= 2.2ms) PASS
  Device Signature: 0x1E 0x94 0x21
  [TX] 50 20                    -> [RX] 14 10                        (RTT= 1.9ms) PASS
[STAGE 2: PASS] Successfully entered programming mode!

=== MODE: READ-ONLY AUDIT (11 pages from 0x0200) ===
Auditing page 1/11 @ 0x0200...
  [TX] 55 00 02 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 88 E0 80 93 21 04 84 E0 80 93 21 04 80 93 26 04 88 E0 80 93 25 04 25 E1 36 E1 95 E0 21 50 30 40 90 40 E1 F7 80 93 26 04 25 E1 36 E1 95 E0 21 50 30 40 90 40 E1 F7 ED CF FF FF FF FF FF FF FF FF 10 (RTT= 7.9ms) PASS
Auditing page 2/11 @ 0x0240...
  [TX] 55 40 02 20              -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 E6 CF 8C E0 80 93 21 04 84 E0 80 93 26 04 88 E0 80 93 25 04 82 E0 80 93 01 04 80 E8 80 93 01 04 80 93 06 04 83 E7 90 E0 80 93 08 08 90 93 09 08 80 EC 80 93 06 08 8E EB 93 E8 0E 94 00 01 8C EE 10 (RTT= 8.0ms) PASS
Auditing page 3/11 @ 0x0280...
  [TX] 55 80 02 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 93 E8 0E 94 00 01 88 E1 94 E8 0E 94 00 01 80 EC 93 E8 0E 94 00 01 10 E0 00 E0 98 E0 C9 2E 2A E0 E2 2E F1 2C 30 E3 B3 2E 40 E8 D4 2E C0 92 27 04 84 E4 94 E8 0E 94 00 01 0F 5F 1F 4F 21 F4 B9 86 10 (RTT= 7.8ms) PASS
Auditing page 4/11 @ 0x02C0...
  [TX] 55 C0 02 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 21 E0 30 E0 1B C0 FE 01 31 96 C8 01 30 E0 20 E0 4F 01 2F 5F 3F 4F B7 01 0E 94 CB 01 80 5D 81 93 CB 01 00 97 B1 F7 FE 01 39 96 C9 01 01 97 D4 01 A8 0F B9 1F 4C 91 41 93 00 97 C1 F7 89 E0 90 E0 10 (RTT= 7.6ms) PASS
Auditing page 5/11 @ 0x0300...
  [TX] 55 00 03 20              -> [RX] 14 10                        (RTT= 2.4ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 8C 0F 9D 1F 28 0F 39 1F F9 01 10 82 CE 01 09 96 0E 94 00 01 80 91 28 04 83 FF 05 C0 84 E6 94 E8 0E 94 00 01 04 C0 87 E7 94 E8 0E 94 00 01 84 E1 88 2E 91 2C 80 91 04 08 87 FF 23 C0 A0 90 00 08 10 (RTT= 7.6ms) PASS
Auditing page 6/11 @ 0x0340...
  [TX] 55 40 03 20              -> [RX] 14 10                        (RTT= 3.9ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 8B E8 94 E8 0E 94 00 01 D0 92 05 04 F9 E0 FA 95 F1 F7 80 91 04 08 85 FF FC CF A0 92 02 08 80 91 04 08 86 FF FC CF 80 91 04 08 80 64 80 93 04 08 89 E0 8A 95 F1 F7 D0 92 06 04 80 EA 94 E8 0E 94 10 (RTT= 8.0ms) PASS
Auditing page 7/11 @ 0x0380...
  [TX] 55 80 03 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 00 01 E2 EC F2 EA 31 97 F1 F7 00 C0 F1 E0 8F 1A 91 08 81 F6 8B CF AA 1B BB 1B 51 E1 07 C0 AA 1F BB 1F A6 17 B7 07 10 F0 A6 1B B7 0B 88 1F 99 1F 5A 95 A9 F7 80 95 90 95 BC 01 CD 01 08 95 0D 0A 10 (RTT= 7.8ms) PASS
Auditing page 8/11 @ 0x03C0...
  [TX] 55 C0 03 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 0D 0A 00 20 20 41 44 58 20 43 6F 72 65 2D 44 20 52 53 2D 34 38 35 20 10 (RTT= 7.8ms) PASS
Auditing page 9/11 @ 0x0400...
  [TX] 55 00 04 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 53 65 72 69 61 6C 20 4D 6F 6E 69 74 6F 72 20 52 65 61 64 79 21 0D 0A 00 20 20 42 61 75 64 72 61 74 65 3A 20 31 31 35 32 30 30 20 62 70 73 20 7C 20 4C 45 44 3A 20 50 42 33 20 28 57 68 69 74 65 10 (RTT= 7.7ms) PASS
Auditing page 10/11 @ 0x0440...
  [TX] 55 40 04 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=303.0ms: <TIMEOUT NO DATA>
  [ERROR] Load address failed for read at 0x0440
!!! HALTED AT PAGE 0x0440 !!!

[STAGE: EXIT] Leaving programming mode...
  [TX] 51 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=302.6ms: <TIMEOUT NO DATA>
[INFO] Serial port closed.
```

===

```powershell

PS C:\Users\User> python debug_flasher.py --port COM19 --hex test_rs485_serial.hex
[INIT] Opening serial port COM19 at 115200 bps...
[INIT] Connected successfully to COM19.

[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 0.00s (probe #1)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 75 20                    -> [RX] 14 1E 94 21 10               (RTT= 2.4ms) PASS
  Device Signature: 0x1E 0x94 0x21
  [TX] 50 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
[STAGE 2: PASS] Successfully entered programming mode!
[HEX] Loaded 676 bytes from test_rs485_serial.hex
[PLAN] Total pages: 11 (0x0200 ~ 0x04C0)

=== MODE: FULL WRITE & VERIFY PER PAGE ===

--- Testing Page 0x0200 (64 bytes) ---
  [TX] 55 00 02 20              -> [RX] 14 10                        (RTT= 2.7ms) PASS
  [TX] 64 00 40 46 20 E8 20 93 05 04 29 E0 2A 95 F1 F7 FC 01 81 91 81 11 10 C0 80 91 04 08 86 FF FC CF 80 91 04 08 80 64 80 93 04 08 89 E0 8A 95 F1 F7 80 E8 80 93 06 04 08 95 90 91 04 08 95 FF FC CF 80 93 02 08 20 -> [RX] 14 10                        (RTT=12.1ms) PASS
  [TX] 55 00 02 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 20 E8 20 93 05 04 29 E0 2A 95 F1 F7 FC 01 81 91 81 11 10 C0 80 91 04 08 86 FF FC CF 80 91 04 08 80 64 80 93 04 08 89 E0 8A 95 F1 F7 80 E8 80 93 06 04 08 95 90 91 04 08 95 FF FC CF 80 93 02 08 10 (RTT= 7.9ms) PASS
  [VERIFY PASS] Page 0x0200 matches perfectly!

--- Testing Page 0x0240 (64 bytes) ---
  [TX] 55 40 02 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 64 00 40 46 E6 CF 8C E0 80 93 21 04 84 E0 80 93 26 04 88 E0 80 93 25 04 82 E0 80 93 01 04 80 E8 80 93 01 04 80 93 06 04 83 E7 90 E0 80 93 08 08 90 93 09 08 80 EC 80 93 06 08 8E EB 93 E8 0E 94 00 01 8C EE 20 -> [RX] 14 10                        (RTT=12.4ms) PASS
  [TX] 55 40 02 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 E6 CF 8C E0 80 93 21 04 84 E0 80 93 26 04 88 E0 80 93 25 04 82 E0 80 93 01 04 80 E8 80 93 01 04 80 93 06 04 83 E7 90 E0 80 93 08 08 90 93 09 08 80 EC 80 93 06 08 8E EB 93 E8 0E 94 00 01 8C EE 10 (RTT= 7.8ms) PASS
  [VERIFY PASS] Page 0x0240 matches perfectly!

--- Testing Page 0x0280 (64 bytes) ---
  [TX] 55 80 02 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 64 00 40 46 93 E8 0E 94 00 01 88 E1 94 E8 0E 94 00 01 80 EC 93 E8 0E 94 00 01 10 E0 00 E0 98 E0 C9 2E 2A E0 E2 2E F1 2C 30 E3 B3 2E 40 E8 D4 2E C0 92 27 04 84 E4 94 E8 0E 94 00 01 0F 5F 1F 4F 21 F4 B9 86 20 -> [RX] 14 10                        (RTT=11.9ms) PASS
  [TX] 55 80 02 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 93 E8 0E 94 00 01 88 E1 94 E8 0E 94 00 01 80 EC 93 E8 0E 94 00 01 10 E0 00 E0 98 E0 C9 2E 2A E0 E2 2E F1 2C 30 E3 B3 2E 40 E8 D4 2E C0 92 27 04 84 E4 94 E8 0E 94 00 01 0F 5F 1F 4F 21 F4 B9 86 10 (RTT= 7.8ms) PASS
  [VERIFY PASS] Page 0x0280 matches perfectly!

--- Testing Page 0x02C0 (64 bytes) ---
  [TX] 55 C0 02 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 64 00 40 46 21 E0 30 E0 1B C0 FE 01 31 96 C8 01 30 E0 20 E0 4F 01 2F 5F 3F 4F B7 01 0E 94 CB 01 80 5D 81 93 CB 01 00 97 B1 F7 FE 01 39 96 C9 01 01 97 D4 01 A8 0F B9 1F 4C 91 41 93 00 97 C1 F7 89 E0 90 E0 20 -> [RX] 14 10                        (RTT=11.8ms) PASS
  [TX] 55 C0 02 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 21 E0 30 E0 1B C0 FE 01 31 96 C8 01 30 E0 20 E0 4F 01 2F 5F 3F 4F B7 01 0E 94 CB 01 80 5D 81 93 CB 01 00 97 B1 F7 FE 01 39 96 C9 01 01 97 D4 01 A8 0F B9 1F 4C 91 41 93 00 97 C1 F7 89 E0 90 E0 10 (RTT= 7.9ms) PASS
  [VERIFY PASS] Page 0x02C0 matches perfectly!

--- Testing Page 0x0300 (64 bytes) ---
  [TX] 55 00 03 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 64 00 40 46 8C 0F 9D 1F 28 0F 39 1F F9 01 10 82 CE 01 09 96 0E 94 00 01 80 91 28 04 83 FF 05 C0 84 E6 94 E8 0E 94 00 01 04 C0 87 E7 94 E8 0E 94 00 01 84 E1 88 2E 91 2C 80 91 04 08 87 FF 23 C0 A0 90 00 08 20 -> [RX] 14 10                        (RTT=12.3ms) PASS
  [TX] 55 00 03 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 8C 0F 9D 1F 28 0F 39 1F F9 01 10 82 CE 01 09 96 0E 94 00 01 80 91 28 04 83 FF 05 C0 84 E6 94 E8 0E 94 00 01 04 C0 87 E7 94 E8 0E 94 00 01 84 E1 88 2E 91 2C 80 91 04 08 87 FF 23 C0 A0 90 00 08 10 (RTT= 7.8ms) PASS
  [VERIFY PASS] Page 0x0300 matches perfectly!

--- Testing Page 0x0340 (64 bytes) ---
  [TX] 55 40 03 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 64 00 40 46 8B E8 94 E8 0E 94 00 01 D0 92 05 04 F9 E0 FA 95 F1 F7 80 91 04 08 85 FF FC CF A0 92 02 08 80 91 04 08 86 FF FC CF 80 91 04 08 80 64 80 93 04 08 89 E0 8A 95 F1 F7 D0 92 06 04 80 EA 94 E8 0E 94 20 -> [RX] 14 10                        (RTT=12.0ms) PASS
  [TX] 55 40 03 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 8B E8 94 E8 0E 94 00 01 D0 92 05 04 F9 E0 FA 95 F1 F7 80 91 04 08 85 FF FC CF A0 92 02 08 80 91 04 08 86 FF FC CF 80 91 04 08 80 64 80 93 04 08 89 E0 8A 95 F1 F7 D0 92 06 04 80 EA 94 E8 0E 94 10 (RTT= 7.8ms) PASS
  [VERIFY PASS] Page 0x0340 matches perfectly!

--- Testing Page 0x0380 (64 bytes) ---
  [TX] 55 80 03 20              -> [RX] 14 10                        (RTT= 3.0ms) PASS
  [TX] 64 00 40 46 00 01 E2 EC F2 EA 31 97 F1 F7 00 C0 F1 E0 8F 1A 91 08 81 F6 8B CF AA 1B BB 1B 51 E1 07 C0 AA 1F BB 1F A6 17 B7 07 10 F0 A6 1B B7 0B 88 1F 99 1F 5A 95 A9 F7 80 95 90 95 BC 01 CD 01 08 95 0D 0A 20 -> [RX] 14 10                        (RTT=12.0ms) PASS
  [TX] 55 80 03 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 00 01 E2 EC F2 EA 31 97 F1 F7 00 C0 F1 E0 8F 1A 91 08 81 F6 8B CF AA 1B BB 1B 51 E1 07 C0 AA 1F BB 1F A6 17 B7 07 10 F0 A6 1B B7 0B 88 1F 99 1F 5A 95 A9 F7 80 95 90 95 BC 01 CD 01 08 95 0D 0A 10 (RTT= 8.0ms) PASS
  [VERIFY PASS] Page 0x0380 matches perfectly!

--- Testing Page 0x03C0 (64 bytes) ---
  [TX] 55 C0 03 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 64 00 40 46 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 0D 0A 00 20 20 41 44 58 20 43 6F 72 65 2D 44 20 52 53 2D 34 38 35 20 20 -> [RX] 14 10                        (RTT=12.1ms) PASS
  [TX] 55 C0 03 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 3D 0D 0A 00 20 20 41 44 58 20 43 6F 72 65 2D 44 20 52 53 2D 34 38 35 20 10 (RTT= 7.7ms) PASS
  [VERIFY PASS] Page 0x03C0 matches perfectly!

--- Testing Page 0x0400 (64 bytes) ---
  [TX] 55 00 04 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 64 00 40 46 53 65 72 69 61 6C 20 4D 6F 6E 69 74 6F 72 20 52 65 61 64 79 21 0D 0A 00 20 20 42 61 75 64 72 61 74 65 3A 20 31 31 35 32 30 30 20 62 70 73 20 7C 20 4C 45 44 3A 20 50 42 33 20 28 57 68 69 74 65 20 -> [RX] 14 10                        (RTT=12.0ms) PASS
  [TX] 55 00 04 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 53 65 72 69 61 6C 20 4D 6F 6E 69 74 6F 72 20 52 65 61 64 79 21 0D 0A 00 20 20 42 61 75 64 72 61 74 65 3A 20 31 31 35 32 30 30 20 62 70 73 20 7C 20 4C 45 44 3A 20 50 42 33 20 28 57 68 69 74 65 10 (RTT= 8.0ms) PASS
  [VERIFY PASS] Page 0x0400 matches perfectly!

--- Testing Page 0x0440 (64 bytes) ---
  [TX] 55 40 04 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=301.9ms: <TIMEOUT NO DATA>
  [ERROR] Load address failed for write at 0x0440
!!! HALTED AT PAGE 0x0440 !!!

[STAGE: EXIT] Leaving programming mode...
  [TX] 51 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=301.3ms: <TIMEOUT NO DATA>

[FINISH] Test session ended.
[INFO] Serial port closed.


```
