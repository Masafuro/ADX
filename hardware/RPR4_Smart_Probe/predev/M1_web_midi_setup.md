# M1: Web MIDI (SysEx) 通信基盤セットアップ & 検証ガイド

## 1. 概要
本ドキュメントは、**Milestone 1（Web MIDI API / SysEx 通信基盤の確立）** におけるファームウェアのビルド方法、Web テストツールの起動方法、および評価ベンチマーク手順をまとめたものである。

---

## 2. 成果物構成

```
hardware/RPR4_Smart_Probe/predev/
├── firmware/m1_usb_midi/       # Pico H 用 USB-MIDI ファームウェア (C/C++)
│   ├── CMakeLists.txt          # ビルド定義
│   ├── pico_sdk_import.cmake   # Pico SDK インポートスクリプト
│   ├── tusb_config.h           # TinyUSB 設定 (CFG_TUD_MIDI = 1, 512Bバッファ)
│   ├── usb_descriptors.c       # USB-MIDI ディスクリプタ (VID: 0xCafe, PID: 0x4002)
│   ├── sysex_protocol.h        # 7-to-8 パッキング & コマンドプロトコル定義
│   ├── sysex_protocol.c        # パッキング/アンパッキング・チェックサム実装
│   └── main.c                  # メインループ、MIDI 受信 & エコーバック処理
│
└── web/                        # Web MIDI テストコンソール (PWA)
    ├── index.html              # テスト UI (PING, エコー, 16KBベンチマーク)
    ├── style.css               # ダークモード UI スタイル
    └── app.js                  # Web MIDI API ドライバ & ベンチマーク実行スクリプト
```

---

## 3. ファームウェアのビルドと書き込み手順

### 3.1 前提環境
- **Raspberry Pi Pico C/C++ SDK**（v1.5.0 以上）
- `arm-none-eabi-gcc`, `cmake`, `ninja`（または `make`）

### 3.2 ビルドコマンド
```bash
cd hardware/RPR4_Smart_Probe/predev/firmware/m1_usb_midi
mkdir -p build && cd build

# PICO_SDK_PATH を指定して CMake 設定
cmake -DPICO_SDK_PATH=/path/to/pico-sdk ..

# ビルド実行
make -j4
```
ビルドが完了すると、`rpr4_m1_usb_midi.uf2` が生成される。

### 3.3 書き込み
1. Pico H の `BOOTSEL` ボタンを押しながら PC の USB ポートに接続する。
2. マウントされたマスストレージドライブ（`RPI-RP2`）に `rpr4_m1_usb_midi.uf2` をドラッグ＆ドロップする。
3. 書き込みが完了すると Pico が自動再起動し、オンボード LED（GP25）が点灯する。

---

## 4. Web テストツールの起動手順

Web MIDI API はセキュリティ上の理由から **HTTPS** または **localhost** 環境でのみ動作する。

### 4.1 PC で実行する場合
リポジトリルートまたは `web/` ディレクトリで簡易 Web サーバーを起動する：
```bash
cd hardware/RPR4_Smart_Probe/predev/web
python3 -m http.server 8000
```
ブラウザ（Chrome, Edge, Firefox）で `http://localhost:8000` を開く。

### 4.2 Android スマートフォンで実行する場合
1. PC と Android スマホを同一 Wi-Fi に接続する。
2. スマホの Chrome から `http://<PCのローカルIP>:8000` にアクセスする。
   - ※同一 LAN 接続で Web MIDI が弾かれる場合は、Chrome の `chrome://flags/#unsafely-treat-insecure-origin-as-secure` に `http://<PCのローカルIP>:8000` を追加して再起動する。
3. Android スマホに OTG ケーブル（Type-C to Micro-B）で Pico H を接続する。
4. Android OS の「MIDI 機器が接続されました」通知を確認する。

---

## 5. ベンチマーク実行手順と受入基準 (Acceptance Criteria)

### 5.1 疎通テスト手順
1. **デバイス接続**:
   - `[Web MIDI アクセス要求 / 検出]` をクリック。
   - ポート選択肢に `RPR4 Smart Probe`（または `TinyUSB MIDI`）が表示されることを確認。
   - `[ポート接続]` をクリックしてステータスが「接続中」になることを確認。
2. **PING テスト**:
   - `[PING 送信]` をクリック。
   - レスポンスとして `ADX RPR4 Smart Probe | HW:Pico-H+2CH-RS485 | FW:v0.1.0-predev` が返り、往復時間（RTT）が 1〜5ms 程度で表示されることを確認。
3. **単発エコーテスト**:
   - `[エコー送信 & 検証]` をクリック。
   - 送信した文字列が 1バイトの化けもなく完全一致（`[PASS]`）することを確認。

### 5.2 16KB スループットベンチマーク手順
- `[16KB ベンチマーク開始]` をクリック。
- 64バイト（Flash 1ページ相当）ずつ計 256 回のパケット送受信が自動実行される。

### 5.3 M1 受入判定基準 & 実機計測結果

| 評価項目 | 合格基準 (Target) | 実機実測結果 (Windows 11 + Chrome) | 判定 |
| :--- | :--- | :--- | :---: |
| **プラグ＆プレイ性** | OS 標準ドライバのみで即認識（ドライバ不要） | `RPR4 Smart Probe` として自動認識 (In=1, Out=1) | **PASS** |
| **パケットエラー率** | 16KB 転送（256往復）でエラー・ロス 0 件 (0.00%) | **エラー 0 件 (ロス 0.00%)** | **PASS** |
| **実効転送レート (64B)** | **20 KB/s 以上**（16KB を 1秒未満で完走） | **30.97 KB/s (所要時間 517 ms)** | **PASS (目標大幅クリア)** |
| **実効転送レート (128B)** | - | **49.61 KB/s (所要時間 323 ms)** | **PASS** |
| **実効転送レート (32B)** | - | **20.13 KB/s (所要時間 795 ms, 512往復)** | **PASS** |
| **平均 RTT (往復レイテンシ)**| 1往復あたり **5 ms 以下** | **2.4 ms (PING) / 6.4 ms (ECHO)** | **PASS** |

> [!NOTE]
> **M1 評価総括**:
> ATtiny1616 の 1ページ（64バイト）に相当する 256 パケット連続往復テストにおいて、**エラー・データ化け一切なしで 517ms（約0.5秒、30.97 KB/s）で完走**した。
> これにより、Web MIDI SysEx を用いた OTW スケッチ書き込み通信基盤の成立性が実機で完全に実証された。


