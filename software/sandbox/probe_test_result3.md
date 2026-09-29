live Telemetry & Hex Dump
Clear
[10:16:52.121]
=== 📈 Delimiter Sweep (復帰時間探索) 開始: 10 条件 x 各 3 回 ===
[10:16:52.125]
固定条件: Break = 2.0ms (Atomic Sync+Frame)
[10:16:55.502]
[Delim 0.1ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:16:58.891]
[Delim 0.5ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:17:02.293]
[Delim 1.0ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:17:05.709]
[Delim 2.0ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:17:08.688]
[Delim 5.0ms] -> 0/3 PASS (rx: 10.3B, Byte[0]: 0x40, Byte[1]: 0x43, RTT: 202.9ms)
[10:17:12.150]
[Delim 10.0ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:17:15.638]
[Delim 15.0ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:17:19.091]
[Delim 20.0ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:17:22.569]
[Delim 30.0ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
[10:17:26.115]
[Delim 50.0ms] -> 0/3 PASS (rx: 0.0B, Byte[0]: --, Byte[1]: --, RTT: 0.0ms)
========================================================================================
                      DELIMITER (復帰時間) AUTOMATED SWEEP REPORT
========================================================================================
 Config         : Break = 2.0ms | Repeats = 3 probes/level
----------------------------------------------------------------------------------------
 Delimiter (ms) |  Result   | rx_count | Byte[0] | Byte[1] | State & Diagnosis
----------------+-----------+----------+---------+---------+----------------------------
 0.1 ms         | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 0.5 ms         | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 1.0 ms         | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 2.0 ms         | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 5.0 ms         | 0/3       | 10.3 B   | 0x40    | 0x43    | ? 不完全 (10.3B, 0x40)
 10.0 ms        | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 15.0 ms        | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 20.0 ms        | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 30.0 ms        | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
 50.0 ms        | 0/3       | 0.0 B    | --      | --      | ✕ TIMEOUT (応答なし)
----------------+-----------+----------+---------+---------+----------------------------
=========================

========================================================================================
                      BREAK PULSE WIDTH AUTOMATED SWEEP REPORT
========================================================================================
 Config         : Delimiter = 10.1ms | Repeats = 3 probes/level
----------------------------------------------------------------------------------------
 Pulse (ms)     |  Result   | rx_count | Byte[0] | Byte[1] | State & Diagnosis
----------------+-----------+----------+---------+---------+----------------------------
 0.7 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
 1.0 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
 1.2 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
 1.5 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
 2.0 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
 2.5 ms         | 0/3       | 10.3 B   | 0x40    | 0x08    | rx:10.3B (0x40)
 3.0 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
 4.0 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
 5.0 ms         | 0/3       | 0.0 B    | --      | --      | rx:0.0B (--)
===============

Live Telemetry & Hex Dump
Clear
[10:20:00.722]
[TX] 🎯 Sending Tuned Half-Baud Re-open Probe (Flush:3ms, Delim:1ms) + Atomic Sync+Frame
[10:20:00.907]
[RX DIAG] Captured: 31B | Byte[0]: 0x40 | Byte[1]: 0x44 (RTT: 179.3ms, CRC OK)
[10:20:00.911]
↳ Slave Rx Buffer Dump [0..15]: 40 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00
[10:20:11.811]
[TX] 🎯 Sending Tuned Half-Baud Re-open Probe (Flush:3ms, Delim:1ms) + Atomic Sync+Frame
[10:20:12.432]
[FAIL] Timeout: Received 0/32 bytes

