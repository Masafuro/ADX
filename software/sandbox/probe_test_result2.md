Live Telemetry & Hex Dump
Clear
[INIT]
Ready. Click 'Connect' to select COM19 @ 19,200 bps.
[09:48:50.463]
Serial port opened successfully @ 19,200 bps.
[09:49:03.528]
[TX] 🎯 Sending Tuned Half-Baud Re-open Probe (Flush:15ms, Rest:12ms) + Atomic Sync+Frame
[09:49:03.934]
[FAIL] Timeout: Received 0/32 bytes
[09:49:14.537]
[TX] Break(2.0ms) + Delim(10.1ms) + Atomic Sync+Frame: 01 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D2 B7
[09:49:14.929]
[FAIL] Timeout: Received 0/32 bytes

★1回目
ーーー
Live Telemetry & Hex Dump
Clear
[09:50:24.431]
=== 🧪 Starting Multi-Pattern Break Explorer (10 Patterns x 3 Probes) ===
[09:50:24.436]
--- Testing P01: Baseline (0.5ms Delim) ---
[09:50:26.342]
[Result] P01: Baseline (0.5ms Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:26.344]
--- Testing P02: Delim 2.0ms (Quick Recover) ---
[09:50:28.287]
[Result] P02: Delim 2.0ms (Quick Recover): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:28.312]
--- Testing P03: Delim 5.0ms (Safe Gap) ---
[09:50:30.282]
[Result] P03: Delim 5.0ms (Safe Gap): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:30.287]
--- Testing P04: Delim 10.0ms (Full Stabilize) ---
[09:50:32.263]
[Result] P04: Delim 10.0ms (Full Stabilize): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:32.269]
--- Testing P05: Delim 20.0ms (Long Delim) ---
[09:50:34.255]
[Result] P05: Delim 20.0ms (Long Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:34.275]
--- Testing P06: Delim 50.0ms (Ultra Delim) ---
[09:50:36.364]
[Result] P06: Delim 50.0ms (Ultra Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:36.372]
--- Testing P07: Short Break 1.0ms + 10ms Delim ---
[09:50:38.335]
[Result] P07: Short Break 1.0ms + 10ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:38.343]
--- Testing P08: Short Break 1.5ms + 5ms Delim ---
[09:50:40.324]
[Result] P08: Short Break 1.5ms + 5ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:40.328]
--- Testing P09: Pre-Lock Writer (Zero Latency) ---
[09:50:42.317]
[Result] P09: Pre-Lock Writer (Zero Latency): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:50:42.325]
--- Testing P10: WebSerial Half-Baud (Tuned 15ms) ---
[09:50:44.164]
[Result] P10: WebSerial Half-Baud (Tuned 15ms): 1/3 PERFECT | rx_cnt: 10.7B | Byte[0]: 0x01 | Byte[1]: 0x01 | Dump[0..7]: 01 01 00 00 00 00 00 00
========================================================================================================
                      MULTI-PATTERN BREAK EXPLORER DIAGNOSIS REPORT
========================================================================================================
 Target MCU     : ATtiny1616 (wu5_diag.hex) on ADX Core-D @ 19,200 bps
 Repeats        : 3 probes per pattern
--------------------------------------------------------------------------------------------------------
 Pattern Name                         | Result | rx_count | Byte[0] | Byte[1] | State & Verdict
--------------------------------------+--------+----------+---------+---------+-------------------------
 P01: Baseline (0.5ms Delim)          | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P02: Delim 2.0ms (Quick Recover)     | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P03: Delim 5.0ms (Safe Gap)          | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P04: Delim 10.0ms (Full Stabilize)   | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P05: Delim 20.0ms (Long Delim)       | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P06: Delim 50.0ms (Ultra Delim)      | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P07: Short Break 1.0ms + 10ms Delim  | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P08: Short Break 1.5ms + 5ms Delim   | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P09: Pre-Lock Writer (Zero Latency)  | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P10: WebSerial Half-Baud (Tuned 15ms) | 1/3    | 10.7 B   | 0x01    | 0x01    | ▲ 準良好 (一部同期成立)
--------------------------------------+--------+----------+---------+---------+-------------------------
 >>> 検証完了: 各パターンの Byte[0] および受信バイト数の変化を確認してください。 <<<
====================================

★2回目
ーーー
Live Telemetry & Hex Dump
Clear
[09:51:46.707]
=== 🧪 Starting Multi-Pattern Break Explorer (10 Patterns x 3 Probes) ===
[09:51:46.710]
--- Testing P01: Baseline (0.5ms Delim) ---
[09:51:48.613]
[Result] P01: Baseline (0.5ms Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:51:48.617]
--- Testing P02: Delim 2.0ms (Quick Recover) ---
[09:51:50.533]
[Result] P02: Delim 2.0ms (Quick Recover): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:51:50.539]
--- Testing P03: Delim 5.0ms (Safe Gap) ---
[09:51:52.518]
[Result] P03: Delim 5.0ms (Safe Gap): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:51:52.525]
--- Testing P04: Delim 10.0ms (Full Stabilize) ---
[09:51:54.496]
[Result] P04: Delim 10.0ms (Full Stabilize): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:51:54.502]
--- Testing P05: Delim 20.0ms (Long Delim) ---
[09:51:56.511]
[Result] P05: Delim 20.0ms (Long Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:51:56.517]
--- Testing P06: Delim 50.0ms (Ultra Delim) ---
[09:51:58.624]
[Result] P06: Delim 50.0ms (Ultra Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:51:58.635]
--- Testing P07: Short Break 1.0ms + 10ms Delim ---
[09:52:00.609]
[Result] P07: Short Break 1.0ms + 10ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:52:00.617]
--- Testing P08: Short Break 1.5ms + 5ms Delim ---
[09:52:02.567]
[Result] P08: Short Break 1.5ms + 5ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:52:02.575]
--- Testing P09: Pre-Lock Writer (Zero Latency) ---
[09:52:04.543]
[Result] P09: Pre-Lock Writer (Zero Latency): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:52:04.557]
--- Testing P10: WebSerial Half-Baud (Tuned 15ms) ---
[09:52:06.525]
[Result] P10: WebSerial Half-Baud (Tuned 15ms): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
========================================================================================================
                      MULTI-PATTERN BREAK EXPLORER DIAGNOSIS REPORT
========================================================================================================
 Target MCU     : ATtiny1616 (wu5_diag.hex) on ADX Core-D @ 19,200 bps
 Repeats        : 3 probes per pattern
--------------------------------------------------------------------------------------------------------
 Pattern Name                         | Result | rx_count | Byte[0] | Byte[1] | State & Verdict
--------------------------------------+--------+----------+---------+---------+-------------------------
 P01: Baseline (0.5ms Delim)          | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P02: Delim 2.0ms (Quick Recover)     | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P03: Delim 5.0ms (Safe Gap)          | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P04: Delim 10.0ms (Full Stabilize)   | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P05: Delim 20.0ms (Long Delim)       | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P06: Delim 50.0ms (Ultra Delim)      | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P07: Short Break 1.0ms + 10ms Delim  | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P08: Short Break 1.5ms + 5ms Delim   | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P09: Pre-Lock Writer (Zero Latency)  | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 P10: WebSerial Half-Baud (Tuned 15ms) | 0/3    | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
--------------------------------------+--------+----------+---------+---------+-------------------------
 >>> 検証完了: 各パターンの Byte[0] および受信バイト数の変化を確認してください。 <<<
=====================================
ーーー

たまに成功するときがある。
ーーー
Total Probes
105
Success Rate
2.9%
Last RTT
0.00 ms
Jitter (σ)
±40.03 ms
●
Live Telemetry & Hex Dump
Clear
[09:53:19.199]
=== 🏆 Starting Half-Baud 20-Cycle Continuous Stress Test (Metronome: 250ms) ===
[09:53:19.592]
[#1/20] FAIL: Timeout: Received 0/32 bytes
[09:53:20.234]
[#2/20] FAIL: Timeout: Received 0/32 bytes
[09:53:20.876]
[#3/20] FAIL: Timeout: Received 0/32 bytes
[09:53:21.540]
[#4/20] FAIL: Timeout: Received 0/32 bytes
[09:53:22.208]
[#5/20] FAIL: Timeout: Received 0/32 bytes
[09:53:22.862]
[#6/20] FAIL: Timeout: Received 0/32 bytes
[09:53:23.521]
[#7/20] FAIL: Timeout: Received 0/32 bytes
[09:53:24.172]
[#8/20] FAIL: Timeout: Received 0/32 bytes
[09:53:24.841]
[#9/20] FAIL: Timeout: Received 0/32 bytes
[09:53:25.519]
[#10/20] FAIL: Timeout: Received 0/32 bytes
[09:53:25.956]
[#11/20] PERFECT (rx:32B, b0:0x01, b1:0x41, RTT:175.8ms)
[09:53:26.622]
[#12/20] FAIL: Timeout: Received 0/32 bytes
[09:53:27.289]
[#13/20] FAIL: Timeout: Received 0/32 bytes
[09:53:27.961]
[#14/20] FAIL: Timeout: Received 0/32 bytes
[09:53:28.620]
[#15/20] FAIL: Timeout: Received 0/32 bytes
[09:53:29.297]
[#16/20] FAIL: Timeout: Received 0/32 bytes
[09:53:29.960]
[#17/20] FAIL: Timeout: Received 0/32 bytes
[09:53:30.617]
[#18/20] FAIL: Timeout: Received 0/32 bytes
[09:53:31.269]
[#19/20] FAIL: Timeout: Received 0/32 bytes
[09:53:31.936]
[#20/20] FAIL: Timeout: Received 0/32 bytes
[09:53:32.195]
=== 🏆 Half-Baud Stress Test Complete: 1/20 Passed (5.0%) ===
