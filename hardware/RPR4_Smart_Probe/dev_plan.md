# RPR4 開発計画

## 概要
当初、構想時は、ラズピコ用のRS-485シールドがうまく手に入らなかった。
現在、スイッチサイエンスの在庫が回復し入手できるようになっている。

よって、基板開発を行う前に、以下の2個を買って
https://www.waveshare.com/wiki/Pico-2CH-RS485
https://www.switch-science.com/products/8170?variant=42382202896582
動作確認はプロトコル実装を行ってみてはどうか？と考えた。

## 懸念点と技術的見通し
- **PIO利用はこのシールドでもできるのか**
  - **回答：可能。** RP2040のGPIOマトリクスにより、PIO0/PIO1からGP0〜GP29の全ピンを制御・サンプリング可能。CH0（GP0/GP1）およびCH1（GP4/GP5）もPIOから直接ドライブ・モニタリングできる。
- **最終的に利用するRS-485トランシーバーやGPIO配置の違い**
  - **注意点：WaveshareシールドはTX連動のハードウェア自動方向制御（Auto Direction）回路を搭載している。**
    - Picoからトランシーバーの `DE` / `~RE` ピンをGPIOで明示的に叩く配線にはなっていない。
    - LIN BREAK（長時間Low）送出や通常の半二重送受信のPoCとしては十分機能するが、本番基板（TDA51S485HCでの明示的DE/RE制御）とターンアラウンドやエコーバック挙動がわずかに異なる可能性がある。
    - ※必要に応じて、DE/REが露出している安価なトランシーバーDIP/モジュールでの補完検証も視野に入れる。

## 目標
- Android OTG + PWA + Pico RS-485 で専用プロトコルによるスケッチ書き込みができるか？
- 同じPWAでPC（Chrome, Edge, Firefox）からもスケッチ書き込みができるか？
  - **技術方針**: **Web MIDI API (SysEx)** を採用（[技術検討メモ](memo/web_midi_protocol_study.md) に基づき正式決定）。
  - **採用理由**:
    - Android Chrome で Polyfill なし・OS 標準ネイティブ動作。
    - Firefox も標準サポート（Web Serial のように拒絶されない）。
    - 40年以上の実績を持つオープン規格であり、OS アップデートによる突然死リスクが極小。
    - 将来の iOS 対応時も Apple の MFi 認証不要（CoreMIDI 経由で極小 Swift ラッパーから利用可能）。
    - RPR4（RP2040）を「インテリジェント・モデム」とし、USB（SysEx）と現場 RS-485（PIO自律波形制御）の責務を完全分離。

## 検証ロードマップ
1. **Phase 1: USB-MIDI 通信基盤の検証 (Pico ⇔ PC/Android PWA)**
   - Pico SDK + TinyUSB (`CFG_TUD_MIDI = 1`) による純粋な USB-MIDI ファームウェアの実装。
   - PWA（Web）から `navigator.requestMIDIAccess({ sysex: true })` によるデバイス検出と SysEx 双方向エコーバック確認。
   - 7-to-8 パッキング処理の実装と、256往復（16KB相当）の転送レイテンシ・スループット計測。
2. **Phase 2: PIO による LIN BREAK & パケット送受信の検証**
   - SysEx コマンド受信をトリガーとし、PIO ステートマシンが高精度 LIN BREAK（13bit+ Dominant Low）+ Sync Byte + LN-485 フレームを自律送出。
   - Waveshare シールド経由での信号波形およびタイミング確認（Auto Direction 回路の挙動検証含む）。
3. **Phase 3: OTW スケッチ書き込みプロトコルの実証**
   - ADX（ATtiny1616）と接続し、1ページ（64B）単位の同期 ACK プロトコルにより、Android PWA / PC から実機へのスケッチ書き込み完走を検証。
4. **Phase 4: シリアルモニタ・テレメトリのストリーミング（拡張）**
   - 書き込み後のターゲットからの UART ログ出力を RPR4 が SysEx で PWA にストリーミング。
   - Web Serial 不可な環境でも、書き込みからログ監視まで完全 Web 完結するスマートプローブ UX を確立。