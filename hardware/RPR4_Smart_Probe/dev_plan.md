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
- 同じPWAでPCからもスケッチ書き込みができるか？
  - **技術方針**: Android版Chromeは標準Web Serial APIが無効なため、**WebUSB API (`navigator.usb`)** または **`web-serial-polyfill`**（WebUSB経由CDC-ACM）を前提としたアーキテクチャで検証する。

## 検証ロードマップ
1. **Phase 1: USB通信基盤の検証 (Pico ⇔ PC/Android PWA)**
   - TinyUSB (CDC-ACM / WebUSB) ファームウェアの導入
   - PWAから `web-serial-polyfill` 経由でAndroid OTG通信およびPC通信の疎通確認
2. **Phase 2: PIOによるLIN BREAK & パケット送受信の検証**
   - PIOステートマシンによる高精度LIN BREAK（13bit+ Dominant Low）+ Sync Byte生成
   - シールド経由での信号波形およびタイミング確認
3. **Phase 3: OTWスケッチ書き込みプロトコルの実証**
   - ADX（ATtiny1616）と接続し、Android PWAから実機へのスケッチ書き込み完走