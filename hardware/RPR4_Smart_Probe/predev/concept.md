# RPR4 Pre development

## 概要
WAVE SHAREの [Pico-2CH-RS485](https://www.waveshare.com/wiki/Pico-2CH-RS485) および Raspberry Pi Pico H を入手した。
RPR4 本番基板（カスタム PCBA）を開発・発注する前に、オフザシェルフのハードウェア環境を用いて、Web MIDI API、PIO 波形制御、および OTW スケッチ書き込みプロトコルの事前検証（PoC）を行う。

## ドキュメント構成
- **[experiment_plan.md](experiment_plan.md)**: ハードウェアおよびファームウェア層を含む実験計画書（マイルストーン M0〜M5、得るべき結果、受入基準）
- **[../dev_plan.md](../dev_plan.md)**: RPR4 全体の開発計画・ロードマップ
- **[../memo/web_midi_protocol_study.md](../memo/web_midi_protocol_study.md)**: Web MIDI API (SysEx) プロトコル選定メモ
