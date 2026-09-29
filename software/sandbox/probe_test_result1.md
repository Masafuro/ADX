1回目


Live Telemetry & Hex Dump
Clear
[INIT]Ready. Click 'Connect' to select COM19 @ 19,200 bps.
[09:24:26.552]Serial port opened successfully @ 19,200 bps.
[09:25:40.227][TX] 🎯 Sending Tuned Half-Baud Re-open Probe (Flush:15ms, Rest:12ms) + Atomic Sync+Frame
[09:25:40.680][FAIL] Timeout: Received 0/32 bytes
[09:25:54.024][TX] Break(2.0ms) + Delim(10.1ms) + Atomic Sync+Frame: 01 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D2 B7
[09:25:54.407][FAIL] Timeout: Received 0/32 bytes
[09:26:20.689]=== 🏆 Starting Half-Baud 20-Cycle Continuous Stress Test (Metronome: 250ms) ===
[09:26:21.130][#1/20] FAIL: Timeout: Received 0/32 bytes
[09:26:21.882][#2/20] FAIL: Timeout: Received 0/32 bytes
[09:26:22.572][#3/20] FAIL: Timeout: Received 0/32 bytes
[09:26:23.271][#4/20] FAIL: Timeout: Received 0/32 bytes
[09:26:24.053][#5/20] FAIL: Timeout: Received 0/32 bytes
[09:26:24.829][#6/20] FAIL: Timeout: Received 0/32 bytes
[09:26:25.595][#7/20] FAIL: Timeout: Received 0/32 bytes
[09:26:26.380][#8/20] FAIL: Timeout: Received 0/32 bytes
[09:26:27.154][#9/20] FAIL: Timeout: Received 0/32 bytes
[09:26:27.911][#10/20] FAIL: Timeout: Received 0/32 bytes
[09:26:28.701][#11/20] FAIL: Timeout: Received 0/32 bytes
[09:26:29.465][#12/20] FAIL: Timeout: Received 0/32 bytes
[09:26:30.225][#13/20] FAIL: Timeout: Received 0/32 bytes
[09:26:31.016][#14/20] FAIL: Timeout: Received 0/32 bytes
[09:26:31.805][#15/20] FAIL: Timeout: Received 0/32 bytes
[09:26:32.590][#16/20] FAIL: Timeout: Received 0/32 bytes
[09:26:33.362][#17/20] FAIL: Timeout: Received 0/32 bytes
[09:26:34.108][#18/20] FAIL: Timeout: Received 0/32 bytes
[09:26:34.906][#19/20] FAIL: Timeout: Received 0/32 bytes
[09:26:35.682][#20/20] FAIL: Timeout: Received 0/32 bytes
[09:26:35.938]=== 🏆 Half-Baud Stress Test Complete: 0/20 Passed (0.0%) ===
[09:26:47.632]=== 🧪 Starting Multi-Pattern Break Explorer (10 Patterns x 3 Probes) ===
[09:26:47.635]--- Testing P01: Baseline (0.5ms Delim) ---
[09:26:49.510][Result] P01: Baseline (0.5ms Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:26:49.513]--- Testing P02: Delim 2.0ms (Quick Recover) ---
[09:26:51.413][Result] P02: Delim 2.0ms (Quick Recover): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:26:51.422]--- Testing P03: Delim 5.0ms (Safe Gap) ---
[09:26:53.348][Result] P03: Delim 5.0ms (Safe Gap): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:26:53.357]--- Testing P04: Delim 10.0ms (Full Stabilize) ---
[09:26:55.298][Result] P04: Delim 10.0ms (Full Stabilize): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:26:55.302]--- Testing P05: Delim 20.0ms (Long Delim) ---
[09:26:57.268][Result] P05: Delim 20.0ms (Long Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:26:57.273]--- Testing P06: Delim 50.0ms (Ultra Delim) ---
[09:26:59.317][Result] P06: Delim 50.0ms (Ultra Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:26:59.326]--- Testing P07: Short Break 1.0ms + 10ms Delim ---
[09:27:01.290][Result] P07: Short Break 1.0ms + 10ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:27:01.296]--- Testing P08: Short Break 1.5ms + 5ms Delim ---
[09:27:03.234][Result] P08: Short Break 1.5ms + 5ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:27:03.250]--- Testing P09: Pre-Lock Writer (Zero Latency) ---
[09:27:05.194][Result] P09: Pre-Lock Writer (Zero Latency): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:27:05.200]--- Testing P10: WebSerial Half-Baud (Tuned 15ms) ---
[09:27:07.115][Result] P10: WebSerial Half-Baud (Tuned 15ms): 1/3 PERFECT | rx_cnt: 10.7B | Byte[0]: 0x01 | Byte[1]: 0x14 | Dump[0..7]: 01 14 00 00 00 00 00 00
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
 P10: WebSerial Half-Baud (Tuned 15ms) | 1/3    | 10.7 B   | 0x01    | 0x14    | ▲ 準良好 (一部同期成立)
--------------------------------------+--------+----------+---------+---------+-------------------------
 >>> 検証完了: 各パターンの Byte[0] および受信バイト数の変化を確認してください。 <<<
=========================================

2回目

Live Telemetry & Hex Dump
Clear
[09:28:48.445]
=== 🧪 Starting Multi-Pattern Break Explorer (10 Patterns x 3 Probes) ===
[09:28:48.448]
--- Testing P01: Baseline (0.5ms Delim) ---
[09:28:50.319]
[Result] P01: Baseline (0.5ms Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:28:50.321]
--- Testing P02: Delim 2.0ms (Quick Recover) ---
[09:28:52.218]
[Result] P02: Delim 2.0ms (Quick Recover): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:28:52.230]
--- Testing P03: Delim 5.0ms (Safe Gap) ---
[09:28:54.139]
[Result] P03: Delim 5.0ms (Safe Gap): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:28:54.146]
--- Testing P04: Delim 10.0ms (Full Stabilize) ---
[09:28:56.086]
[Result] P04: Delim 10.0ms (Full Stabilize): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:28:56.090]
--- Testing P05: Delim 20.0ms (Long Delim) ---
[09:28:58.069]
[Result] P05: Delim 20.0ms (Long Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:28:58.076]
--- Testing P06: Delim 50.0ms (Ultra Delim) ---
[09:29:00.116]
[Result] P06: Delim 50.0ms (Ultra Delim): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:29:00.119]
--- Testing P07: Short Break 1.0ms + 10ms Delim ---
[09:29:02.037]
[Result] P07: Short Break 1.0ms + 10ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:29:02.045]
--- Testing P08: Short Break 1.5ms + 5ms Delim ---
[09:29:03.974]
[Result] P08: Short Break 1.5ms + 5ms Delim: 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:29:03.982]
--- Testing P09: Pre-Lock Writer (Zero Latency) ---
[09:29:05.922]
[Result] P09: Pre-Lock Writer (Zero Latency): 0/3 PERFECT | rx_cnt: 0.0B | Byte[0]: -- | Byte[1]: -- | Dump[0..7]:
[09:29:05.934]
--- Testing P10: WebSerial Half-Baud (Tuned 15ms) ---
[09:29:08.193]
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
=================================
