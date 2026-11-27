<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-U (rev5)：「Android フレンドリー」技術・ビルド性・Play ストア実現性調査レポート

- **作成日**: 2026-11-27
- **ステータス**: 調査・フィージビリティ検証レポート（rev5 / EAA 構想向け）
- **対象構想**: [EAA_3_friendly_concept.md](EAA_3_friendly_concept.md) / [20261127_concept.md](20261127_concept.md)
- **関連仕様・既存資産**:
  - rev5 技術仕様案: [spec_proposal.md](spec_proposal.md)
  - 既存 Android 試作資産: [adx-break-probe-android/README.md](../../../software/sandbox/adx-break-probe-android/README.md)
  - 既存 Android 計画書: [ADX_MR32_FLASHER_ANDROID_PLAN.md](../../../software/sandbox/ADX_MR32_FLASHER_ANDROID_PLAN.md)

---

## 0. エグゼクティブサマリー（結論）

> **結論：「Android 上で Arduino IDE（オンデバイス `avr-gcc` ビルド）を作らない」という一線さえ守れば、Android フレンドリーのソフト開発・Play ストア運用は十分に軽く、現実的に成立する。**

1. **ビルド性の核心：オンデバイス `avr-gcc` は 100% 沼（絶対避けるべき）**
   - Android 10+（`targetSdk >= 29`）の **W^X 制限（`execve` 禁止）**、Play ストアの **API 36 義務（2026/8/31〜）**、**16KB ページサイズ義務（2025/11〜）** により、Android 端末内でローカルコンパイラを動かす構成は Termux ですら Play ストア撤退に追い込まれた地雷原。
   - 代わりに、**「Android はコンパイルしない（＝B パターン `AI-Firmata` 操作盤・電力アナライザ・ロガー・`.hex` の UPDI 書込/強制復旧機に徹する）」** と定義すれば、開発負荷は Arduino IDE 級と比べ **2 桁軽く** なる。
   - 将来どうしても「スマホ単体でコード修正→ビルド→書込」をやりたくなったら、オンデバイスではなく **クラウドコンパイル API（サーバー側で 1 秒ビルドし `.hex` だけ返す）** で W^X 制限を完全に回避できる。
2. **Play ストア ＆ 配布環境の最新現実（2026 年末〜2027 年）**
   - Google の **Android Developer Verification Program**（2026 年 9 月先行開始 → **2027 年 1 月グローバル展開**）により、GitHub 等の **野良 APK 配布でも開発者本人確認・署名登録が実質必須化**（未検証 APK はブロック＋24 時間待機）。「Play ストアは避けて野良 APK でいい」は今後の一般向け戦略として成り立ちにくい。
   - 一方で **Chrome 148（2026 年 5 月）で Android 版 Web Serial API が正式リリースされたが、ここには重要な注意点がある**。Android 16（API 36）以下では依然として **Bluetooth RFCOMM のみ** であり、有線 USB シリアルは **Android 17（API 37）で新設される OS 標準の `android.hardware.serial`（`SerialManager` / `native_serial`）が必須** となる。したがって「PWA のみで有線対応」できるのは **Android 17 以降の普及後** の世界線となる。
3. **既存自社資産がそのまま最強の勝ち筋になる**
   - リポジトリ既存の [adx-break-probe-android](../../../software/sandbox/adx-break-probe-android) は、**純粋 Kotlin わずか 410 行（ネイティブ `.so` ゼロ）＋ 共通 HTML/JS（WebView ＋ `android-serial-shim.js`）** で USB-Serial 実機動作をすでに達成している。
   - **ネイティブ `avrdude` バイナリを同梱せず、`ATtiny1616` 専用に絞った純粋 JS/Kotlin のミニ SerialUPDI フラッシャー（約 200〜300 行）** を組むことで、**W^X 問題・16KB ページサイズ問題・PC/Android 二重開発のすべてを一撃で回避** できる。

---

## 1. ビルド性の問題：Android でファームをどう扱うか（4 パターン徹底比較）

「下手すると Arduino IDE のような Android アプリを作る羽目になるのでは？」という懸念に対し、4 つのアーキテクチャを比較検証した。

| パターン | 構成概要 | Play ストア適合 | 開発・保守負荷 | 判定 |
| :--- | :--- | :---: | :---: | :---: |
| **① オンデバイス `avr-gcc`**<br>*(ArduinoDroid / Termux 型)* | Android 内に `avr-gcc` + `binutils` + `megaTinyCore` を入れて端末内でビルド | **✕（極めて困難）** | **極重（沼）** | **✕ 不採用** |
| **② ブラウザ内 WASM ビルド**<br>*(Clang/LLVM-AVR WASM)* | WebView/ブラウザ内で WASM 化した LLVM-AVR により C/C++ をコンパイル | ○（JS/WASM のみ） | 重（最適化・SDK 仮想 FS 保守） | **△ 時期尚早** |
| **③ クラウドビルド API ＋ Android UPDI 書込** | Android からコードを送信 → クラウド上の `arduino-cli` が 1 秒ビルド → `.hex` を返却 → Android が UPDI 書込 | **◎（完全準拠）** | 中（サーバー運用費・認証あり） | **○ Phase 2 候補** |
| **④ ビルドなし（HMI ＋ 電力計 ＋ ロガー ＋ `.hex` UPDI フラッシャー）** | 開発ビルドは PC（AI）担当。Android は `AI-Firmata` 直接操作・計測・ログ・既製/PC 作成 `.hex` 書込・強制復旧に専念 | **◎（完全準拠）** | **極軽（既存資産の延長）** | **◎ 最推奨（MVP）** |

### 1.1 なぜパターン ①（オンデバイス `avr-gcc`）が沼なのか
1. **Android 10+（`targetSdk >= 29`）の W^X 制限（`execve` 禁止）**：  
   Android 10 以降、`targetSdk >= 29` のアプリは自アプリのデータ領域（`/data/data/...`）に展開した実行バイナリを `execve()` で起動することが SELinux レベルで禁止されている。**Termux** すらこの仕様変更により Play ストア版の更新を事実上放棄し、`targetSdk 28` に固定した F-Droid / GitHub 配布へ撤退した。
2. **`jniLibs` 静的同梱ハックの地獄**：  
   `avr-gcc` 群を `lib*.so` にリネームして APK 内の読み取り専用領域へ静的同梱する回避策はあるが、① APK サイズが 100〜250MB 超へ肥大、② **2025 年 11 月から必須化された「16KB メモリページサイズ対応」** のため全ネイティブ実行バイナリを ARM64 向けに独自再ビルド・継続保守する羽目になる、③ ユーザーによるライブラリ追加ができない、という三重苦になる。
3. **先行アプリの実態**：  
   Play ストアの `ArduinoDroid` は定番 AVR 向けには動いても、`megaTinyCore`（ATtiny1616）＋ UPDI のようなモダン構成は公式対応が手薄になりがちである。一方、**書込専用の `ZFlasher AVR`（`avrdude 7.3+` で ATtiny1616 の SerialUPDI 書込が実績あり）** や **通信専用の `Serial USB Terminal`** は何年も安定して Play ストアで生き残っている。

### 1.2 パターン ②（WASM コンパイラ）の現状
- `avr-gcc` 自体の WASM 移植はビルド構造上ほぼ非現実的。
- `Clang / LLVM`（AVR バックエンド）の WASM 化（例：`wasm-clang` 系）は技術的に可能だが、① LLVM の AVR バックエンドは `avr-gcc` に比べ `-Os` のサイズ最適化が弱く、**16KB Flash の `ATtiny1616` では容量圧迫リスクが高い**、② WASM バイナリ本体（数十 MB）と `megaTinyCore` ヘッダ群のメモリ展開（`MEMFS`）のロード負荷・保守負荷が重く、CORE-U 初期フェーズで背負うべきコストではない。

### 1.3 パターン ④（MVP）→ パターン ③（拡張）が最強である理由
- **CORE-U の「大脳と脊髄の分離」と 100% 整合する**：  
  コードを書いてビルドする本格開発（A パターン）は **PC + AI エージェント（VS Code / Antigravity）** に任せ、Android は **「現場の目（V/I/W 電力計）・手（`AI-Firmata` タッチ HMI）・記憶（128GB ロガー）・救急箱（UPDI 強制リセット＆`.hex` 書き換え）」** に特化する。
- 将来「PC を持たずに Android だけで AI に話しかけてファームを書き換えたい」ニーズが顕在化したら、Android アプリ側は **1 行もコンパイラを持たず**、クラウド上の極小 `arduino-cli` コンテナへソースを投げて数 KB の `.hex` を受け取り、既存の UPDI フラッシャーで焼くだけ（パターン ③）で完結できる。

---

## 2. Google Play ストア ＆ Android 配布環境の最新状況（2026 年基準）

### 2.1 Google Play ストアの主要要件とハードル

| 要件項目 | 2026 年時点のルール | CORE-U Android アプリへの影響と対策 |
| :--- | :--- | :--- |
| **Target SDK 義務** | **2026 年 8 月 31 日以降**、新規・更新アプリは **Android 16（API 36）** 以上必須（既存アプリの検索可視性は API 35 以上）。 | 既存試作（`targetSdk = 34`）を `36` へ更新。年 1 回の追従保守が必要だが、純粋 Kotlin＋WebView 構成なら数時間で済む。 |
| **16KB メモリページサイズ義務** | **2025 年 11 月以降**、Android 15+ ターゲットの新規・更新アプリでネイティブライブラリ（`.so`）の 16KB アライメント対応が必須。 | **`avrdude` 等の C/C++ ネイティブバイナリを同梱せず、純粋 Kotlin + JS（`.so` ゼロ）にすれば完全無傷で自動クリア！** |
| **新規個人アカウントのクローズドテスト義務** | 2023/11/13 以降作成の**個人アカウント**は、公開前に **12 人以上のテスター × 14 日間連続オプトイン＆利用実績** が必須（当初 20 人から 12 人へ緩和）。**法人（組織）アカウント（D-U-N-S 取得）は免除**。 | 個人公開する場合はコミュニティ等で 12 人集める計画が必要。事業・製品として継続配布するなら **法人アカウント取得が最も摩擦が少ない**。 |
| **実行コード動的 DL 禁止ポリシー** | Play ストア外から実行コード（ネイティブ/DEX、アプリ挙動を大幅改変するリモート JS 等）を動的取得する仕組みはリジェクト対象になりやすい。 | Web UI（HTML/JS/CSS）は外部 URL 読み込みではなく **APK 内の `assets/` へローカル同梱** することで安全にクリア。 |
| **USB Host 権限審査** | `android.hardware.usb.host` のみを使用し、位置情報・Bluetooth・カメラ・広域ストレージ権限を要求しない構成。 | プライバシー高リスク権限を踏まないため、**BLE 連携アプリ（位置情報/Nearby 権限が煩雑）よりむしろ審査が通りやすい**。 |

### 2.2 「野良 APK（GitHub 配布）」は今後どうなるか（2026〜2027 年の重要変化）
- Google は **Android Developer Verification Program** を導入（**2026 年 9 月 30 日**にブラジル・インドネシア・シンガポール・タイで先行開始、**2027 年 1 月よりグローバル展開**）。
- これにより、Play ストア外（GitHub Releases や Web サイト直配布）の APK であっても、**Google の開発者コンソールで本人確認・パッケージ名・署名鍵を紐付けていない未検証 APK は Play Protect にブロック** される方向へ移行している（解除には開発者オプション有効化＋**24 時間クールダウン待機**という一般ユーザーに不可能な手順が必要。ADB インストールや限定台数ホビー枠は例外）。
- **示唆**：  
  「Play ストアは面倒だから GitHub で野良 APK を配ればいい」という逃げ道は 2027 年以降に急激に狭まる。**最初から Google 開発者アカウントで署名登録し、Play ストア正規公開をゴールに据える**（または Web Serial を併用する）設計が安全。

### 2.3 Web アプリ配布（PWA ＋ Android Web Serial API）の正確な現実：**Chrome 148 だけでは有線は動かない（Android 17 必須）**
- **表面的な情報と一次ソースのギャップ**：
  - **Chrome 148（2026 年 5 月リリース、ChromeStatus `#6043992171085824`）** で Android 版の Web Serial API が正式リリースされたのは事実だが、リリースノート上の API 概要説明（「USB や Bluetooth 経由で接続…」）から「Chrome 148 だけで有線 USB も動く」と誤解されやすい。
  - **一次ソース（Chromium Intent to Ship / プラットフォーム実装）の正確な仕様**：
    1. **Android 16（API 36）以下**：Chrome 148+ でも **Bluetooth RFCOMM（SPP）のみ**。有線 USB シリアルは OS 側のシステムサービス不在（Chromium ログ上 `"Android Serial Service requires SDK 37"` / `"Could not find Android Serial Service"`）により列挙されない（2026 年秋の実機検証で有線ドングルが見えなかった現象と完全一致）。
    2. **Android 17（API 37）以降**：OS 側に新設された **`android.hardware.serial`（`SerialManager` ＋ Rust 製 `native_serial` デーモン）** により、初めて **Chrome 148+ × Android 17+ の組み合わせでブラウザ単体（PWA）からの有線 USB シリアル** が動く前提となる。
- **「Chrome Android Web Serial 普及待ち（PWA 一本化）」に対する現実的評価**：
  1. **定着まで約 5 年スパンの不確実性**：Android 17 搭載端末の普及、ベンダーごとの実装安定化、さらには Web から物理ポートを叩くことに対するセキュリティインシデントやブラウザポリシー見直しを越えて許容され続けるかが見極まるには、今後 **5 年程度の時間軸** が必要。
  2. **「引き出しの古い余剰 Android 端末（Android 7.0〜16）」を完全に逃す**：GOT として最も魅力的な余剰スマホ資産は Android 17 へアップデートされないため、Web Serial 一本化では対象外になってしまう。
  3. **Android 17 実機での `CH342K` 互換も未確定**：`native_serial` が Dual CDC-ACM や UPDI の `8E2` / Double-Break をどこまで透過的に扱えるかは今後の検証次第。

> **配布戦略の最終結論：「当面は薄型オフライン `ADX GOT Shell` APK を主力とし、共通 HTML で将来に備える」**  
> - **今すぐ（主力）**：Android 7.0〜17 の全端末で確実に動く **純粋 Kotlin 約 600 行・`.so` ゼロ・`INTERNET` 権限なしの超軽量 `ADX GOT Shell` APK** で、「EA ＋ Android GOT」をビルドサーバーゼロで成立させる。  
> - **将来（5 年スパンの保険）**：GOT 画面を標準 HTML/CSS/JS（`got.html`）＋ PC 共通のシリアルブリッジ API で設計しておくことで、将来 Android Chrome Web Serial が完全に成熟した際にも、同じ `got.html` 資産がそのままブラウザ単体で動く構造を維持する。

---

## 3. なにができて、なにができないか（機能・UX の完全仕分け）

`Android スマホ + USB-C 1 本 + CORE-U rev5 (CH342K + ATtiny1616)` で実現可能な範囲を明確に線引きする。

### 3.1 ◎ 軽開発で確実にできること（Phase 1：MVP 範囲）
| カテゴリ | 具体的にできること | 使用ポート・方式 |
| :--- | :--- | :--- |
| **リアルタイム操作（HMI）** | 工場出荷 `AI-Firmata` 経由で、コンパイル 0 秒の RGB LED 制御・GPIO・PWM・ADC 読取・I2C スキャン・SPI 送受信・`VBUS_EN`（10W バス）ON/OFF | `Port 1` (UART) |
| **ポータブル電力アナライザ** | `VBUS_VSENSE` / `VBUS_ISENSE` からの電圧（V）・電流（A）・電力（W）・`EFUSE_FLG`・MCU 温度・VDD を Android の 6 インチ画面でリアルタイム波形グラフ表示 | `Port 1` (UART) |
| **大容量データロガー** | 取得したテレメトリやセンサー値を Android 側のストレージへ CSV/JSON で長時間保存、そのままメール/Slack/Google Drive へ共有（オンボード Flash 不要） | `Port 1` (UART) + Android ストレージ |
| **暴走時のワンタップ強制復旧** | スケッチ暴走・フリーズ時でも、箱を開けず物理ボタンに触れずに `SerialUPDI` 経由でハードウェア強制リセット | `Port 0` (SerialUPDI: `8E2` + Double-Break) |
| **現場でのファーム書き換え** | PC（AI）でビルドした `.hex` ファイル、またはアプリ同梱のプリセット（工場出荷 `AI-Firmata.hex` への復元等）を現場で 2 秒書き込み | `Port 0` (SerialUPDI) |
| **簡易ノーコード自動化** | Android 側 JS で数十〜数百 ms 周期の条件ルールを実行（例：「2.0A 超過で `VBUS_EN` 遮断＋警告音」「特定時刻に動作」） | `Port 1` (UART) |

### 3.2 △ 条件付きでできること（Phase 2：工夫・追加設計が必要）
- **スマホ単体での AI コード生成 → ビルド → 書込**：  
  アプリ内から LLM API でコード生成 → クラウド上の軽量 `arduino-cli` API で 1 秒ビルド → `.hex` を DL して `Port 0` で UPDI 書込。**オフラインでは動かない**＋サーバー運用費とサンドボックスセキュリティ管理が必要。
- **画面消灯・バックグラウンドでの 24 時間連続ロギング**：  
  Android の Doze モード／バックグラウンド停止を避けるため、**Foreground Service**（常駐通知表示＋API 34+ の `foregroundServiceType="connectedDevice"` 宣言＋バッテリー最適化除外案内）の実装が必要。
- **余剰 Android を 24 時間常設のタッチパネル HMI として固定運用**：  
  `USB-COM` 自体は Android を充電しないため、市販の **USB-C PD パススルー充電対応ミニハブ** を挟んで「スマホへ充電しつつ CORE-U と USB 通信」する配線ガイドが必要。
- **ブラウザ単体（PWA）での有線 USB 接続・UPDI 書込**：  
  **Android 17（API 37：`android.hardware.serial`）＋ Chrome 148+** の環境が前提。`Port 1` の通常 UART（8N1）より `Port 0` の UPDI（`8E2`＋300bps Double-Break）のほうがブラウザ・OS 依存が出やすいため、Android 17 実機での検証が必要。

### 3.3 ✕ できないこと／やるべきでないこと（明確な撤退ライン）
- **Android 端末内オフラインでのフル C++ / Arduino IDE ビルド**：W^X 制限・Play 規約・16KB ページサイズ・保守コストの観点で割に合わない。
- **iPhone（iOS）との有線 USB-C 接続**：CDC-ACM / DriverKit 非対応のため MFi チップなしでは不可（EAA では潔く対象外とする）。
- **ミリ秒未満（< 1ms）のハードリアルタイム制御ループを Android 側で回すこと**：USB バルク転送遅延・Android OS スケジューリング揺らぎがあるため、高速 PID やモーター相制御は必ず ATtiny1616 側へ A パターンで書き込む。
- **Android スマホのバッテリーからの 10W 外部負荷給電**：スマホ保護のため `USB-PWR` への外部給電（AC アダプタまたはモバイルバッテリー）を必須前提とする。

---

## 4. 開発・保守コストの現実的見積り（なぜ「軽く」抑えられるのか）

リポジトリ内の既存試作 [adx-break-probe-android](../../../software/sandbox/adx-break-probe-android) のコード規模を実測すると、Android ネイティブ側の実態は驚くほど小さい。

| ファイル | 役割 | 既存実測行数 | CORE-U rev5 向けの改修内容 |
| :--- | :--- | :---: | :--- |
| `MainActivity.kt` | WebView 起動・ライフサイクル | **73 行** | ほぼ流用（ファイル選択 `.hex` 読み込み対応を数十行追加） |
| `UsbSerialBridge.kt` | `usb-serial-for-android` ラッパー | **337 行** | **2 ポート同時管理**（`driver.ports[0]` / `ports[1]`）＋ **`8E2` パリティ/ストップビット設定**対応へ拡張（+100〜150 行程度） |
| `device_filter.xml` | USB 自動検知フィルタ | **17 行** | すでに `0x1A86` (WCH) と `class="2"` (CDC-ACM) 記載済み！ |
| `build.gradle.kts` | 依存関係・SDK 設定 | **55 行** | `compileSdk`/`targetSdk` を `34` → **`36`** へ引き上げ（純粋 Kotlin・`.so` ゼロ） |
| `assets/*` (HTML/JS) | UI・電力グラフ・`AI-Firmata`・UPDI 書込 | 共通資産 | **PC Web と Android APK で 100% 共有**。`ATtiny1616` 専用の純粋 JS SerialUPDI 実装（約 200〜300 行）をここに配置 |

### 設計上の最大のキモ：「`avrdude` を使わず、ATtiny1616 専用ミニ UPDI を JS で書く」
- 汎用 AVR 書込器（`ZFlasher AVR`）が重くなる理由は、数十種の MCU と書込器に対応した C 製 `avrdude` バイナリを NDK で抱えているため。
- 一方、CORE-U（および CORE-A5/A24）のターゲットは **ATtiny1616（tinyAVR 1-series / NVMCTRL v0）の 1 種類のみ** であり、SerialUPDI プロトコル自体は極めてシンプル（300bps Double-Break → `0x55` 同期 → `NVMPROG` 鍵送信 → 64B ページ単位書込 → `ASI_RESET_REQ`）。
- これを **純粋 JavaScript（または Kotlin）で約 200〜300 行** で実装してしまえば：
  1. **ネイティブ `.so` がゼロ**（W^X・16KB ページサイズ・ABI 複数対応が全部不要）。
  2. **APK サイズは 5〜6MB の超軽量を維持**。
  3. **PC ブラウザ（Web Serial）と Android アプリ（WebView）でまったく同じ UPDI フラッシャーが動く**。
  4. 毎年の Play ストア保守も **年 1 回の `targetSdk` 引き上げ確認のみ** で、個人・少人数体制でも十分持続可能。

---

## 5. 結論と段階的ロードマップ

**「Android フレンドリー」は、IDE を作ろうとしなければ決して重すぎない。**  
むしろ既存資産（Kotlin 410 行＋共通 Web UI）の延長線上で、他社ボードには真似できない「10W 電力計＋タッチ HMI＋大容量ロガー＋箱を開けない現場 UPDI 復旧機」を実現できる極めてコスパの高い柱になる。

> **最終方針の確定（2026-11-27 追記）：「EA ＋ Android GOT」路線を採用**  
> 本調査の結果を踏まえ、Android フレンドリーの正体を **「IDE（開発端末）」ではなく「GOT（Graphic Operation Terminal：グラフィック操作盤）」** として定義した（詳細：[EAA_3_friendly_concept.md](EAA_3_friendly_concept.md)）。  
> - **ビルドサーバー・オンデバイスコンパイラ不要**（開発は PC＋AI、Android は AI が生成した `got.html` を読み込んで表示・操作・ログ・非常リセットを行う専用 GOT プレイヤーに専念）。  
> - **Android 17 Web Serial の 5 年単位の普及・安定化待ちに依存せず**、既存 `adx-break-probe-android`（Kotlin 410 行・`.so` ゼロ）を拡張した **100% オフライン（`INTERNET` 権限なし）の薄型 `ADX GOT Shell` APK（約 600 行）** で、Android 7.0〜最新まで 10 年分の余剰端末を今すぐ産業用 GOT として活用する。

### 推奨フェーズ計画（EA ＋ Android GOT 路線）

1. **Phase 0（実機フィージビリティ検証：1〜2 日）**
   - 手元の `ATtiny1616 + CH342K` 基板と手持ちの Android 端末（旧端末含む）を使い、以下 3 点を実測確認する：
     1. **`CH342K` の `VDD5`（VBUS 受電）／`VIO`（ボード 3V3）分離配線** での PC ＆ Android OTG ホットプラグ安定性（[EAA_3_friendly_concept.md §6.1](EAA_3_friendly_concept.md#61-ハードウェア最重要見直しch342k-の-vdd5--vio-電源分離android-otg-対策)）。
     2. `usb-serial-for-android`（`CdcAcmSerialDriver`）での `CH342K`（`1A86:55D2`）**Port 0 / Port 1 同時オープン**と、Port 0 からの純 JS/Kotlin **SerialUPDI 強制リセット＆16KB 書込**（`8E2` + 300bps Double-Break）。
     3. PC Chrome（Web Serial）と Android WebView で、**同一の単一 `got.html`** が 1 文字も変えずに動く共通シリアルブリッジ API の疎通。
2. **Phase 1（CORE-U rev5 ローンチ：`EA` ＋ `ADX GOT Shell` MVP）**
   - **ハードウェア**：`CH342K` `VDD5/VIO` 分離を盛り込んだ rev5 基板を出荷。
   - **PC 側（AI 開発）**：VS Code / Antigravity（`adx-cli` / `adx-mcp`）＋ AI が ATtiny1616 ファームと `got.html` を同時生成するテンプレート／Skill。
   - **Android 側（`ADX GOT Shell`）**：既存 [adx-break-probe-android](../../../software/sandbox/adx-break-probe-android) をベースにした **オフライン専用（`INTERNET` 権限なし）・`.so` ゼロ・Kotlin 約 600 行の Play ストア正規 APK**（標準同梱の電力計・シリアルモニタ画面 ＋ SAF ローカル `got.html` 読込・自動復元 ＋ フルスクリーン常時点灯 ＋ 隠し Port 0 UPDI 復旧）。
   - ※ Play ストア公開は 12 人テスト義務を回避でき 2027 年以降の APK 署名要件にも強い **組織（法人）アカウント** を推奨。
3. **Phase 2（長期・環境成熟後の拡張オプション）**
   - 将来（目安 3〜5 年スパン）Android 17+ の `android.hardware.serial` と Chrome Web Serial が広く普及・安定しセキュリティ面も定着した段階で、同じ `got.html` 資産をブラウザ（PWA）単体でもそのまま動かせる導線を追加（それまではネイティブ薄型シェルを主力とする）。
   - 常設ロガー需要が出た場合のみ、Foreground Service（画面 OFF 連続記録）を追加。
