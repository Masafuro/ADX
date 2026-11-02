# RPR4 Pre development

## 概要
WAVE SHAREの [Pico-2CH-RS485](https://www.waveshare.com/wiki/Pico-2CH-RS485) および Raspberry Pi Pico H を入手した。
RPR4 本番基板（カスタム PCBA）を開発・発注する前に、オフザシェルフのハードウェア環境を用いて、Web MIDI API、PIO 波形制御、および OTW スケッチ書き込みプロトコルの事前検証（PoC）を行う。

## ドキュメント構成
- **[experiment_plan.md](experiment_plan.md)**: ハードウェアおよびファームウェア層を含む実験計画書（マイルストーン M0〜M5、進捗管理表）
- **[M0_hardware_bringup.md](M0_hardware_bringup.md)**: M0 ハードウェア初期確認 & Auto-direction 回路特性レポート (✅ PASS)
- **[M1_web_midi_setup.md](M1_web_midi_setup.md)**: M1 Web MIDI (SysEx) 通信基盤手順書 & 実機計測結果 (✅ PASS: 31〜50 KB/s)
- **[M2_pio_verification.md](M2_pio_verification.md)**: M2 PIO による LIN BREAK 送出 & 8B LN-485 ループバック検証レポート (✅ PASS: 1,000 pkt / ロス 0.00%)
- **[firmware/m2_pio_rs485/memo/20261102_uart_rx_fifo_investigation.md](firmware/m2_pio_rs485/memo/20261102_uart_rx_fifo_investigation.md)**: UART RX FIFO 32B 境界値調査と 8B フレーム採用決定メモ
- **[../dev_plan.md](../dev_plan.md)**: RPR4 全体の開発計画・ロードマップ
- **[../memo/web_midi_protocol_study.md](../memo/web_midi_protocol_study.md)**: Web MIDI API (SysEx) プロトコル選定メモ
