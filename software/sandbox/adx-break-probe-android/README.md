# ADX Break Probe for Android

ADX Core-D BR32 ブートローダ用 **Web BREAK Explorer** の Android 専用 APK プロジェクトです。  
Android 版 Chrome で有線 USB-Serial が非対応である制約を解決し、**USB-OTG 経由でハードウェア BREAK 信号（TX 強制 L レベル）および 19,200bps UART 双方向通信をネイティブ実行**します。

---

## 1. 特徴

- **WebView + ネイティブ USB ブリッジ**:
  [WU5_1_web_break_probe/index.html](../../../../firmware/bootloader/PLAN/BR32/WU/WU5_1_web_break_probe/index.html) の全 UI（スライダー、コンソール、グラフ、自動スイープ探索）を 100% 活用。
- **低レイヤー BREAK 信号生成**:
  `usb-serial-for-android` のネイティブ API（`port.setBreak(true/false)`）を使用し、OS の制限を受けずに確実なハードウェア BREAK パルスを生成。
- **プラグ＆プレイ自動認識**:
  CH340 (0x1A86), FTDI (0x0403), CP210x (0x10C4), PL2303, CDC-ACM 等の主要 USB-シリアルチップを挿入時に自動検知。
- **デュアル動作互換**:
  `assets/android-serial-shim.js` により、PC Chrome では通常の WebSerial API、Android アプリ内ではネイティブ USB ドライバを透過的に自動切り替え。

---

## 2. ビルド方法

### 方法 A: Ubuntu CLI で直接ビルドする場合
```bash
cd software/sandbox/adx-break-probe-android
./build_apk.sh
```
ビルド完了後、以下のパスに Debug APK が生成されます：
`app/build/outputs/apk/debug/app-debug.apk` (約 5.5 MB)

### 方法 B: Android Studio（Windows / Mac / Linux）で開く場合
1. Android Studio を起動し、「Open」から本ディレクトリ (`software/sandbox/adx-break-probe-android`) を選択。
2. Gradle 同期完了後、ツールバーの「Run 'app'」または `Build > Build APK(s)` をクリック。

---

## 3. 実機へのインストールと使い方

### 3.1 インストール
#### パソコンから adb 経由でインストールする場合:
```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

#### スマホ単体でインストールする場合:
1. `app-debug.apk` を Google Drive、Slack、USB メモリ等でスマートフォンに転送。
2. スマホのファイルマネージャー等から APK をタップし、「提供元不明のアプリのインストールを許可」してインストール。

### 3.2 使い方
1. スマートフォンに **USB-OTG アダプタ（Type-C 変換等）** を装着。
2. USB-UART 変換ドングル（CH340, CP2102, FT232R 等）を挿入。
3. Android OS のダイアログ **「ADX Break Probe に USB デバイスへのアクセスを許可しますか？」** が表示されたら **「OK」**（または常に許可にチェック）をタップ。
4. 画面ヘッダーのステータスが **`CONNECTED (19200)`（緑色インジケータ）** に変化すれば接続完了。
5. 「PROBE SINGLE BREAK」や各種スイープボタンを実行して、BR32 ブートローダの探索を行います。
