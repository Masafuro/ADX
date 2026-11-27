<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-U (rev5)：「EAA（3 フレンドリー）」コンセプト構想書
〜 **Enclosure（筐体）× AI Agent（AI）× Android GOT（グラフィック操作盤）** 〜

> **Built for 3D-Printed Enclosures, AI Agents, and Android GOT.**  
> **「Android フレンドリー」とは、IDE としてではなく、GOT（Graphic Operation Terminal）として Android にフレンドリーであること。**

- **作成日**: 2026-11-27（Android GOT 再定義版）
- **ステータス**: 構想確定案（rev5 上位コンセプト定義）
- **着想元メモ**: [20261127_concept.md](20261127_concept.md)
- **関連ドキュメント**:
  - rev5 技術仕様案: [spec_proposal.md](spec_proposal.md)
  - Android 実現性・Play ストア検証: [android_friendly_feasibility.md](android_friendly_feasibility.md)
  - AI フレンドリー構想: [CORE-U_AI_friendly_microcontroller_board.md](../CORE-U_AI_friendly_microcontroller_board.md)
  - 市場予備調査: [AI_friendly_device_market_pre_review.md](../AI_friendly_device_market_pre_review.md)
  - 設計哲学背景: [2026-11-26_craftsmanship_is_not_an_optimization_problem.md](../../../memo/99_SMOKINGROOM/2026-11-26_craftsmanship_is_not_an_optimization_problem.md)
  - ADX 20P バス規格: [ADX_PINOUT.md](../../../docs/spec/ADX_PINOUT.md)
  - Android 実装先行資産: [adx-break-probe-android/README.md](../../../software/sandbox/adx-break-probe-android/README.md)

---

## 1. 核心コンセプト：なぜ「EAA（3 フレンドリー）」なのか

2005 年に誕生した Arduino は、「手打ちコーディングとブレッドボードによる実験」をフレンドリーにした。  
それから 21 年が経過した 2026 年現在、私たちの身の回りには **2005 年には存在しなかった巨大な大衆インフラ** が普及している。

「平均的優等生」ではなく **「特定の生態系に異様に噛み合った特化型」として生き残る**（*How to survive?*）視点で環境変化を棚卸しすると、他社がすでに獲った領域と、**Arduino UNO / Raspberry Pi Pico / M5Stack のどれもが獲れていない 3 つの空白** が鮮明に浮かび上がる。

### 1.1 2005 → 2026 の環境変化と生態系の棲み分け

| 2005 → 2026 で身の回りに新しく普及したもの | 既存プレイヤーの獲得状況 | ADX CORE-U (rev5) の戦略判断 |
| :--- | :--- | :--- |
| **Wi-Fi / Bluetooth（無線 IoT）** | **ESP32 / M5Stack が完全制覇** | **戦わない**（無線・アンテナ・技適・発熱を潔く捨てる） |
| **大容量 Flash / 高速エッジ処理** | **Raspberry Pi Pico / ESP32-S3 が優勢** | **戦わない**（オンボード Flash は載せず、拡張 CARD または GOT 側へ委譲） |
| **① 3D プリンター**（個人筐体製造） | UNO / Pico / M5 とも本質的に未対応 | **獲得 → `E`: Enclosure-Friendly（筐体フレンドリー）** |
| **② AI エージェント**（自律開発・UI 生成） | 物理ボタン・1 ポート競合・盲目で既存機は詰む | **獲得 → `A`: AI Agent-Friendly（AI フレンドリー）** |
| **③ 余剰 Android スマホ / 安価タブレット** | 10W 電源分離 × USB-C 1 本の GOT 運用は完全空白 | **獲得 → `A`: Android GOT-Friendly（Android 操作盤フレンドリー）** |

> **なぜ「Android IDE」ではなく「Android GOT」なのか？**  
> 工場の制御盤では、**PLC（制御の脊髄）** と **GOT（三菱電機由来の産業用語：Graphic Operation Terminal＝タッチパネル表示器・操作盤）** の役割が完全に分かれている。GOT の上でラダー図をコンパイルする現場は存在しない。  
> 引き出しに眠る古い Android スマホの真価は「開発端末（IDE）」ではなく **「USB-C 1 本で繋がる超高精細カラータッチ操作盤（GOT）」** にある。IDE を諦めて GOT に振り切ることで、**オンデバイスコンパイラ（W^X 制限の沼）も、ビルドサーバー運用も、Android 17 Web Serial の 5 年単位の普及待ちも、すべて不要になる**。

---

## 2. EAA（3 フレンドリー）の詳細定義

### 2.1 `E`：Enclosure-Friendly（筐体フレンドリー）
- **背景インフラ**：家庭用 3D プリンター（Bambu Lab 等）の普及により、電子工作が「机の上の裸試作」から「3D プリント筐体へ組み込んだ実用装置（Functional Print）」へ移行した。
- **既存ボードの限界**：UNO は変則寸法と抜けるピンソケット、Pico はブレッドボード専用＋物理 BOOTSEL ボタン必須、M5Stack は既存ケース固定かつ ESP32 の発熱で密閉 PLA 筐体が変形する。
- **CORE-U rev5 の実装**：
  - **`8858` 固定フォームファクター**（88 × 58 mm、四隅 M3 取付穴、AI が OpenSCAD 等で筐体を一発自動生成できる定型規格）。
  - **完全ボタンレス**：密閉ケースにリセット穴や BOOTSEL 穴を開ける必要ゼロ。
  - **低発熱・無線レス**：ATtiny1616（3.3V / 10MHz）の極小発熱で PLA や内部センサーに悪影響を与えず、金属筐体でも問題なく稼働。
  - **ADX 20P IDC 一本化**：振動で抜けない確実な拡張接続。さらに **3D プリンターで「8858 CORE-U ＋ 古い Android スマホをはめ込むベゼル筐体」を印刷すれば、そのまま産業機械のフロントパネルが完成する**。

### 2.2 `A`：AI Agent-Friendly（AI フレンドリー）
- **背景インフラ**：VS Code / Antigravity 等の AI コーディングエージェントの普及により、「人間が IDE で手打ちして目と指でデバッグする」前提が崩壊した。
- **既存ボードの限界**：暴走時の物理リセット必須、1 ポート競合、電圧・電流など物理データの盲目。
- **CORE-U rev5 の実装**：
  - **A / B パターン 2 秒往復**：工場出荷 `AI-Firmata`（B パターン：箱出し 0 秒の MCP 直接操作＋`WS2812B` による *AI's First Blink*）と、`adx-cli` による自律ビルド＆UPDI 強制焼込（A パターン）を物理操作ゼロで往復。
  - **`CH342K` 独立 2 ポート**：`Port 0`（SerialUPDI：暴走しても 2 秒で強制自己修復）と `Port 1`（UART：常時監視・対話）の完全分離。
  - **`ATtiny1616` の決定論的透明性**：データシート全体を AI が把握でき、1 秒ビルド・ハルシネーションゼロ。
  - **AI の目と手（パワー可観測性＆安全制御）**：`VBUS_EN`（初期安全 OFF・強制パワーサイクル）、`VBUS_VSENSE` / `VBUS_ISENSE`（V/I/W リアルタイム計測）、`EFUSE_FLG`、内蔵 VDD・温度を I2C 非占有で標準装備。
  - **★ EAA ならではのキラーシナジー**：**AI は ATtiny1616 のファームウェアと同時に、Android GOT 用の操作画面（単一 `got.html`）まで 10 秒で同時生成できる**。

### 2.3 `A`：Android GOT-Friendly（Android 操作盤フレンドリー）
- **背景インフラ**：世界中の引き出しに眠っている「数年前の余剰 Android スマホ」や、1 万円前後の安価な Android タブレットという莫大な遊休タッチパネル資産。
- **なぜ iPhone を切り、Android に特化するのか**：  
  iPhone は USB-C 化後も標準 USB シリアル（CDC-ACM / DriverKit）を一般開放しておらず、MFi 認証チップなしでは有線接続できない。一方 Android は Android 7.0（API 24）以降の膨大な端末で root 不要・ドライバ不要の USB-Host シリアル通信が確立している。
- **既存 HMI / 表示器プラットフォームとの比較（破壊的コスパ）**：

| プラットフォーム | 価格目安 | 画面・操作性 | 画面の作り方（AI 親和性） | 電源・負荷制御 |
| :--- | :---: | :--- | :--- | :--- |
| **産業用 GOT（三菱等）** | 5 万〜15 万円 | 高信頼タッチパネル | 高額な専用ソフト（GT Designer 等）で手動配置 | 別途 PLC＋電源盤 |
| **Nextion HMI 液晶** | `$40〜$100` | 3.5–5" 感圧式・低解像度 | Windows 専用 GUI エディタで手動設計（**AI 不可**） | 別途 MCU＋電源 |
| **M5Stack CoreS3 等** | `$55〜$60` | 2" 極小液晶 | C++ 描画コードを手書き・ESP32 負荷大 | LiPo 前提・10W 不可 |
| **ADX CORE-U rev5 ＋ 余剰 Android GOT** | **本体 `$17` ＋ 余剰スマホ `$0〜$30`** | **6" フル HD 有機 EL 静電容量タッチ・音・振動** | **標準 HTML/CSS/JS（`got.html` 1 枚）を AI が 10 秒で自動生成！** | **10W 電源分離・eFuse・V/I/W 計測内蔵** |

- **CORE-U rev5 ハードウェアが Android GOT に最適化されている 4 つの理由**：
  1. **10W 電源分離によるスマホ完全保護**：UNO や Pico を Android に挿すとスマホの電池を吸い、モーター起動でスマホの USB ポートが落ちる。CORE-U rev5 は `USB-PWR`（5V 2A）と `USB-COM` が分離しているため、**スマホに負荷をかけず 10W 装置を安全に制御・計測できる**。
  2. **標準 Dual CDC-ACM（`CH342K`: `1A86:55D2`）の 2 ポート活用**：`Port 1` で GOT 画面と単純テキスト／JSON-Lines 通信をしつつ、システム隠しメニューから `Port 0`（SerialUPDI）で **箱を開けずに暴走 MCU の強制リセットや `.hex` 復旧** ができる。
  3. **オンボード V/I/W 電力計の即時可視化**：繋いだ瞬間から、6 インチ画面のポータブル電力アナライザ＆異常監視盤になる。
  4. **データ保存の完全委譲**：ログは Android GOT 側の GB 級ストレージへ CSV/JSON 保存できるため、マイコン側に無理やり Flash を積む必要がない。

---

## 3. 全体アーキテクチャ：「PC（AI 開発）→ CORE-U（小型 PLC）↔ Android GOT（操作盤）」

開発（IDE / AI）と運用（GOT / 現場表示器）を明確に分離し、1 本の `USB-COM` ポートに繋ぎ替えるだけで 3 つの形態を自在に行き来する。

```text
   【開発・作画フェーズ（机上）】        【現場 GOT 運用フェーズ（操作盤）】     【完全自律フェーズ】
  ┌───────────────────────────┐       ┌─────────────────────────────┐       ┌─────────────────┐
  │ Host PC + AI エージェント  │       │ 余剰 Android スマホ / タブレット│       │  USB-COM 抜去   │
  │ ・ATtiny1616 ファーム生成  │       │  [ ADX GOT Shell (オフライン) ]│       │ (スタンドアロン) │
  │ ・UPDI 自動ビルド＆焼込   │       │  ・AI 生成 got.html を表示    │       └─────────────────┘
  │ ・got.html を 10 秒で生成 │──────▶│  ・6" タッチ HMI / V・I・W 計  │                │
  │   (PC Chrome で即プレビュー)│ ファイル│  ・CSV ログ保存               │                │ (未接続)
  └─────────────┬─────────────┘  転送  │  ・隠しメニューで UPDI 復旧    │                │
                │ USB-C 1 本           └──────────────┬──────────────┘                │
                └──────────────────┬──────────────────┘                               │
                                   ▼                                                  ▼
                  ┌────────────────────────────────────────────────────────┐
                  │      ADX CORE-U (rev5) : 10W 物理脊髄 / 小型 PLC ($17帯) │
                  │  ・CH342K (Dual CDC: Port 0 UPDI / Port 1 GOT・UART)   │
                  │  ・ATtiny1616 (3.3V / 10MHz / 低発熱 / ボタンレス)       │
                  │  ・USB-PWR 5V 2A (10W) + eFuse + VBUS_EN + V/I/W 監視  │
                  │  ・8858 筐体規格 (3D プリント筐体＆GOT ベゼル一体収納)   │
                  └───────────────────────────┬────────────────────────────┘
                                              │ ADX 20P IDC (耐振リボン 1 本)
                                              ▼
                                 [ 各種 ADX 拡張 CARD (10W 負荷/センサー) ]
                                 ※ 同じ設計資産のまま産業用 CORE-A へ直結
```

### 3.1 引き算の設計が EAA で 100% 強みに反転する構造

| CORE-U が「載せなかったもの」 | 単体ボード視点での弱点 | **EAA（筐体 × AI × Android GOT）視点での必然性** |
| :--- | :--- | :--- |
| **画面・タッチパネルなし** | M5Stack より単体で地味 | 余剰 Android の 6 インチ有機 EL を GOT にすれば良く、本体を `$17` 台・薄型・低発熱に保てる |
| **Wi-Fi / BLE なし** | IoT 単体機として使えない | 密閉金属筐体でも使え、技適・発熱ゼロ。ネット連携が必要なら GOT 側や外部ゲートウェイへ分離 |
| **オンボード Flash / SD なし** | 大容量ログが取れない | GOT（Android）側の GB 級ストレージに保存可能。単体長期ログは拡張 CARD へ委譲 |
| **物理ボタンなし** | トラブル時に手動リセット不可 | 3D プリント密閉筐体に穴が不要。PC（AI）からも Android GOT からも `Port 0 (UPDI)` で強制リセット可能 |
| **8-bit / 16KB MCU** | Pico / ESP32 に計算力で負ける | 高度な UI 描画は Android GOT、設計は PC（AI）が担う。脊髄は「極小・低発熱・決定論的・CORE-A 共通」が正義 |

---

## 4. `ADX GOT Shell`（超薄型 Android シリアル通信アプリ層）の設計指針

Android 側に必要なソフトウェアは **「IDE ではなく、任意の HTML（`got.html`）を読み込んでシリアル通信するだけの超薄型オフライン GOT プレイヤー」** である。  
リポジトリ既存の [adx-break-probe-android](../../../software/sandbox/adx-break-probe-android)（純粋 Kotlin 410 行 ＋ WebView ＋ `android-serial-shim.js`）をそのまま発展させ、**Kotlin 合計約 600 行（ネイティブ `.so` ゼロ・APK 5〜6MB）** で完結させる。

### 4.1 設計の 4 原則（Play ストア審査 ＆ 現場信頼性の両立）
1. **`INTERNET` パーミッションすら持たない 100% オフライン設計**  
   - 外部サーバーから実行コードを落とす余地を OS レベルで完全に断つことで、Play ストアの「動的コード実行／セキュリティ」審査リスクとプライバシー審査負荷を極小化する。
   - 工場・実験室のオフライン現場でも一切ネットワーク依存なく確実に動作する。
2. **ユーザー主導のローカル `.html` 読み込み（Android SAF: `ACTION_OPEN_DOCUMENT`）**  
   - PC で AI が生成・プレビュー済みの単一 `got.html` を Android へ転送し、アプリ側のファイルピッカーから読み込んで WebView に表示する。
   - 前回開いた `got.html` をアプリ内へ保存し、**次回起動時は即座に操作盤画面へ自動復帰** する。
3. **標準 GOT 画面を APK 内に必ず同梱**  
   - 初期状態（カスタム HTML 未読込時・Play ストア審査時の実機なし起動）でも、デモモード付きの標準 GOT 画面（V/I/W 電力アナライザ・RGB 操作・`VBUS_EN` スイッチ・シリアルモニタ）が動くようにし、「最小機能不足」リジェクトを防ぐ。
4. **PC Chrome（Web Serial）と Android GOT で 1 文字も変えない共通 JS API**  
   - 既存 `android-serial-shim.js`（Web Serial 互換）に加え、AI が最小コードで書ける行ベース API（例：`window.adxGot.sendLine(str)` / `window.adxGot.onLine = (line) => ...`）を提供。
   - PC ブラウザ上で AI と対話しながら `got.html` をライブ開発・テストし、完成したファイル 1 枚をそのまま古い Android へ渡せば 100% 同じ画面が動く。

### 4.2 産業用 GOT らしい実用機能（極小コストで効く要素）
- **キオスク風フルスクリーン（Immersive Mode）＋ 画面常時点灯（`FLAG_KEEP_SCREEN_ON`）**。
- **隠しシステムメニュー（画面端長押し等）に `Port 0 (UPDI)` 緊急機能を隔離**：通常の `got.html` からは触れない安全設計としつつ、現場でマイコンがハングした際だけ管理者が 1 タップで **強制ハードウェアリセット** や **ローカル `.hex` 書込（ATtiny1616 専用・純 JS 約 250 行）** を実行できる。
- **常設給電ガイド**：一時的な保守・操作は USB-C ケーブル 1 本直結、24 時間常設パネルにする場合は市販の USB-C 充電対応 OTG スプリッタ／PD パススルーハブ併用を標準構成とする。

---

## 5. 設計判断記録（Decision Record）

迷いを繰り返さないため、検討の過程で明確に切り捨てた 3 つの選択肢と根拠を記録する。

1. **オンボード Flash（データフレンドリー）の不採用**  
   - `ATtiny1616` はピン残数 0 本、SPI Flash は CS 不足と 4KB セクタ vs 2KB SRAM の壁、ADX バス共有時は `VBUS_EN = OFF` クランプ問題が発生、24P 化（`ATtiny3217`）は売価 `$20` 超過と在庫薄を招く。
   - 設定値等は内蔵 **EEPROM 256B + `USERROW` 32B（`$0`）**、大容量ログは **Android GOT 側ストレージ** または **専用ロガー CARD** へ分離する。
2. **Android 上での IDE / オンデバイスビルド / 初手ビルドサーバーの不採用**  
   - オンデバイス `avr-gcc` は Android 10+ W^X 制限・16KB ページサイズ義務・巨大 APK の沼（Termux が Play ストア撤退した領域）。
   - Android を **GOT（操作盤）専用** と定義することで、オンデバイスビルドもクラウドビルドサーバーの構築・維持費も初手から一切不要になる。
3. **Chrome Android Web Serial 一本化（普及待ち）の早期依存回避**  
   - Chrome 148（2026/5）で Android Web Serial がリリースされたものの、有線 USB は **Android 17（API 37: `android.hardware.serial`）必須** であり、Android 16 以下の膨大な余剰スマホでは動かない。
   - 仮に Android 17 以降で動いたとしても、端末普及・動作安定化・セキュリティインシデントを経た仕様定着には **今後 5 年程度** を見込むのが現実的。当面は **`usb-serial-for-android` ベースの超薄型ネイティブ GOT シェル（Android 7.0〜17 対応）** を正攻法とし、将来 Web Serial が完全に成熟したときに同じ `got.html` がブラウザ単体でもそのまま動く保険にしておく。

---

## 6. rev5 ハードウェア必須チェックポイント ＆ Next Steps

### 6.1 ハードウェア最重要見直し：`CH342K` の `VDD5` / `VIO` 電源分離（Android OTG 対策）
- **課題**：現行 [spec_proposal.md](spec_proposal.md) の `CH342K` セルフパワー接続（ボード側 `3V3` 常時通電）は、`CH342K`（ESSOP-10）に VBUS 検知ピンがないため `UD+` プルアップが出っ放しになり、Android OTG ホットプラグ時に認識エラー（`error -71` 等）を起こすリスクがある。
- **対策（追加コスト `$0`）**：CH342 データシート §5.2 の独立電源・逆流防止仕様を活かし、
  - **`VDD5`（Pin 4）**：`USB-COM` の VBUS（5V、消費 10〜15mA のみ。10W 負荷・MCU 電源とは完全分離）から受電
  - **`V3`（Pin 6）**：内蔵 LDO 出力（0.1µF デカップリングのみ）
  - **`VIO`（Pin 7）**：CORE-U 本体の `3V3`（`USB-PWR` 側 LDO）へ接続
  とすることで、Android へのホットプラグ認識を USB 2.0 準拠で安定させつつ、主電源 OFF 時の MCU への回り込み給電も防ぐ（`VIO = 0V` 時の CH342K 内蔵低電圧リセット挙動のみ実機確認する）。

### 6.2 4 段階 UX ジャーニー（EAA 完成形）
1. **Stage 1（AI フレンドリー）**：箱出し 60 秒。PC の AI から `AI-Firmata` で物理操作ゼロの *AI's First Blink* と V/I/W 観測。
2. **Stage 2（筐体フレンドリー）**：AI に自律ファームと 8858 3D プリント筐体を生成させ、UPDI 焼込 → `USB-COM` を抜いて単体稼働。
3. **Stage 3（Android GOT フレンドリー）**：AI に 10 秒で作らせた `got.html` を引き出しの古い Android（`ADX GOT Shell`）へ読み込み、USB-C 1 本で接続。**`$17` 基板＋余剰スマホで、タッチ操作盤・電力アナライザ・ロガー・箱を開けない UPDI 復旧機能付きの本格装置が完成**。
4. **Stage 4（産業連続性）**：同じ ATtiny1616・同じ 20P バス・同じ 8858 筐体・同じ Android GOT のまま、過酷現場用 `CORE-A` へシームレス昇格。

### 6.3 Next Steps
1. **ハード実機確認（1〜2 日）**：手元の `ATtiny1616 + CH342K` 基板と古い Android 端末を繋ぎ、① `VDD5`（VBUS）/ `VIO`（3V3）分離での OTG ホットプラグ安定性、② `usb-serial-for-android` での `CH342K` 2 ポート同時認識、③ Port 0 からの純 JS/Kotlin SerialUPDI 強制リセットを検証。
2. **[spec_proposal.md](spec_proposal.md) の rev5 最終化**：`CH342K` の `VDD5/VIO` 分離配線と EAA（Android GOT）の位置づけを仕様書へ反映し、rev5 アートワーク／試作へ進む。
3. **`ADX GOT Shell` 最小 PoC（既存資産流用）**：[adx-break-probe-android](../../../software/sandbox/adx-break-probe-android) をベースに「ローカル `got.html` 読み込み＋標準電力計画面＋Port 1 通信＋隠し Port 0 リセット」の最小版を動かし、AI 生成 `got.html` が 10 秒で Android 操作盤になるデモを実機確認する。
