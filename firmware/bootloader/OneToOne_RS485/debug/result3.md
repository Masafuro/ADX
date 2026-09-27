```powershell

PS C:\Users\User> python debug_flasher.py --port COM19 --read-only --start 0x0440 --pages 3
[INIT] Opening serial port COM19 at 115200 bps...
[INIT] Connected successfully to COM19.

[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 5.17s (probe #43)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 75 20                    -> [RX] 14 1E 94 21 10               (RTT= 2.2ms) PASS
  Device Signature: 0x1E 0x94 0x21
  [TX] 50 20                    -> [RX] 14 10                        (RTT= 2.1ms) PASS
[STAGE 2: PASS] Successfully entered programming mode!

=== MODE: READ-ONLY AUDIT (3 pages from 0x0440, delay=20ms) ===
Auditing page 1/3 @ 0x0440...
  [TX] 55 40 04 20              -> [RX] 14 10                        (RTT= 2.2ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 29 0D 0A 00 5B 41 44 58 20 43 6F 72 65 2D 44 5D 20 48 65 61 72 74 62 65 61 74 20 70 61 63 6B 65 74 20 23 00 20 7C 20 57 68 69 74 65 20 4C 45 44 3A 20 4F 4E 0D 0A 00 20 7C 20 57 68 69 74 65 20 10 (RTT= 8.0ms) PASS
Auditing page 2/3 @ 0x0480...
  [TX] 55 80 04 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 4C 45 44 3A 20 4F 46 46 0D 0A 00 3E 3E 3E 20 45 63 68 6F 20 72 65 63 65 69 76 65 64 3A 20 27 00 27 0D 0A 00 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 10 (RTT= 7.8ms) PASS
Auditing page 3/3 @ 0x04C0...
  [TX] 55 C0 04 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 10 (RTT= 7.9ms) PASS

[STAGE: EXIT] Leaving programming mode...
  [TX] 51 20                    -> [RX] 14 10                        (RTT= 1.9ms) PASS
[INFO] Serial port closed.

```

```powershell

PS C:\Users\User> python debug_flasher.py --port COM19 --read-only --start 0x0400 --pages 2
[INIT] Opening serial port COM19 at 115200 bps...
[INIT] Connected successfully to COM19.

[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 4.69s (probe #39)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 1.9ms) PASS
  [TX] 75 20                    -> [RX] 14 1E 94 21 10               (RTT= 2.4ms) PASS
  Device Signature: 0x1E 0x94 0x21
  [TX] 50 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
[STAGE 2: PASS] Successfully entered programming mode!

=== MODE: READ-ONLY AUDIT (2 pages from 0x0400, delay=20ms) ===
Auditing page 1/2 @ 0x0400...
  [TX] 55 00 04 20              -> [RX] 14 10                        (RTT= 2.3ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 53 65 72 69 61 6C 20 4D 6F 6E 69 74 6F 72 20 52 65 61 64 79 21 0D 0A 00 20 20 42 61 75 64 72 61 74 65 3A 20 31 31 35 32 30 30 20 62 70 73 20 7C 20 4C 45 44 3A 20 50 42 33 20 28 57 68 69 74 65 10 (RTT= 7.7ms) PASS
Auditing page 2/2 @ 0x0440...
  [TX] 55 40 04 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=301.9ms: <TIMEOUT NO DATA>
  [ERROR] Load address failed for read at 0x0440

  [HEALTH CHECK] Probing if Core-D is still alive in bootloader...
  [TX] 30 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=302.4ms: <TIMEOUT NO DATA>
  [TX] 30 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=301.2ms: <TIMEOUT NO DATA>
  [TX] 30 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=301.1ms: <TIMEOUT NO DATA>
  [HEALTH: DEAD] Core-D did NOT respond to sync probes. It likely reset or crashed.
!!! HALTED AT PAGE 0x0440 !!!

[STAGE: EXIT] Leaving programming mode...
  [TX] 51 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=302.5ms: <TIMEOUT NO DATA>
[INFO] Serial port closed.


```