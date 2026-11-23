<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ARLE.cc / ADX 公式ドキュメントサイト（Astro Starlight）設計・構成案

- **文書 ID**: `PLAN-ARLE-SITE-001`
- **作成日**: 2026-11-23
- **ステータス**: 承認済・M1 実装中
- **対象**: `site/`（`https://arle.cc` / Cloudflare Pages 配信）

---

## 1. 目的と基本方針

`Reference CARD v0`、`Prototype CARD v0`、`ADX CORE-U` といった実機基板の登場を見据え、GitHub リポジトリ（400 件以上の開発メモ・アーカイブが同居する「実験室」）とは別に、外部ユーザーが迷わず仕様・ピンアウト・設計リファレンスへ到達できる **公式ドキュメント＆ボード図鑑ポータル** を `https://arle.cc` に一本化して構築するための設計案です。

### 合意済み前提
1. **配信基盤**: 本リポジトリの `site/` に直接構築し、既存の Cloudflare Pages（`arle.cc`）から静的 HTML として自動ビルド・配信する。
2. **技術選定**: **Astro 5 ＋ Starlight**（Node 20 / 22 両対応 / 静的サイト生成 / Pagefind 日本語全文検索 / 未翻訳フォールバック内蔵）。
3. **言語方針**: **英語をデフォルト（`/`）**、**日本語を併設（`/ja/`）** とする。
4. **URL プレフィックス**: ADX コンテンツは **`/adx/` 配下**（日本語は `/ja/adx/` 配下）に配置し、将来 ARLE.cc ブランド配下に別製品ラインが増えても綺麗に拡張できる構造とする。
5. **執筆規律**: [ADX トーン＆マナー ガイドライン](../ADX_TONE_AND_MANNER.md) の「誠実な工学主義」および「仕様（英語 SSOT）と解説の疎結合」に準拠する。

---

## 2. 公開チャネルの役割分担

「公開仕様・ユーザー向け窓口」と「研究開発の作業場」を明確に切り分けます。

| チャネル | ソース場所 | 配信先 | 役割 |
| :--- | :--- | :--- | :--- |
| **公式ブランド＆Docs サイト** | `site/` | Cloudflare Pages (`https://arle.cc`) | ブランド LP、ボード図鑑、Arduino 風ピンアウト図、筐体／回路設計者リファレンス |
| **GitHub リポジトリ** | リポジトリ全体 | GitHub (`Masafuro/ADX`) | 回路・ガーバー・ファームウェア開発、`docs/spec/` 仕様 SSOT、`memo/`・`archive/` 開発ログ |
| **WebSerial ツール群** | `docs/index.html`, `docs/flasher/` | GitHub Pages (`masafuro.github.io/ADX/`) | BREAK Explorer・RS-485 Web Flasher（**現状維持・本計画の影響なし**） |
| **開発ブログ / DevLog** | `docs/blog/` | Hackaday.io + jsDelivr CDN | 週次開発ログ・試行錯誤ストーリー（**現状維持・本計画の影響なし**） |

---

## 3. 情報設計（IA）と URL マップ

ADX の強みである **「箱に入れやすい（Enclosure-Native）」** という思想をサイト構造そのものに反映し、通常の「Hardware / Software」ではなく **「ボード図鑑 × 筐体設計者向け × 回路設計者向け」** の 3 本柱をトップ階層に配置します。

```text
https://arle.cc/                              (日本語: /ja/)
│  ARLE.cc トップページ（Starlight splash レイアウト / ダーク工業デザイン）
│  ・ブランドメッセージ + ADX エコシステムへの入口カード
│
└── /adx/                                     (日本語: /ja/adx/)
    ├── /adx/                                 … ADX Overview（概要・はじめかた）
    │
    ├── /adx/hardware/                        (日本語: /ja/adx/hardware/)
    │   ├── /adx/hardware/                    … ボード一覧・選び方マトリクス
    │   ├── /adx/hardware/core-u/             … ADX CORE-U（USB 標準コア）
    │   ├── /adx/hardware/reference-card/     … ADX Reference CARD (v0 / 8858)
    │   └── /adx/hardware/prototype-card/     … ADX Prototype CARD (v0 / 8854※注記)
    │
    ├── /adx/enclosure/                       (日本語: /ja/adx/enclosure/)
    │   ├── /adx/enclosure/                   … 筐体設計者向けガイド（要点まとめ）
    │   ├── /adx/enclosure/8858-formfactor/   … 8858 外形・M3座面・ガイド・光学マーカー
    │   ├── /adx/enclosure/racks-and-cases/   … PLC縦挿しラック・市販筐体(W=70等)・3Dプリント指針
    │   └── /adx/enclosure/downloads/         … 寸法図 SVG・STEP / DXF ダウンロード（順次追加）
    │
    ├── /adx/circuit/                         (日本語: /ja/adx/circuit/)
    │   ├── /adx/circuit/                     … 回路・CARD 設計者向けガイド（要点まとめ）
    │   ├── /adx/circuit/adx-pinout/          … ADX 20-Pin Bus v4.2 仕様（SSOT）＋ ピンアウト図
    │   ├── /adx/circuit/pinout-rationale/    … ピン配置の設計思想・シールド・PoL 原則解説
    │   ├── /adx/circuit/mcu-mapping/         … ATtiny1616 / CH32X035 マッピング（SSOT）
    │   ├── /adx/circuit/guidelines/          … フォームファクタ開発ガイドライン（MUST/SHOULD/MAY）
    │   └── /adx/circuit/optical-id/          … 光学アライメント＆DataMatrix Board ID 生成ガイド
    │
    ├── /adx/software/                        (※M4 以降で有効化)
    │   ├── /adx/software/arduino-ide/        … Arduino IDE 1.8.x + megaTinyCore セットアップ
    │   ├── /adx/software/adx-header/         … ADX_<BOARD>.h（ADX_P01〜P10 マクロ規約）
    │   └── /adx/software/web-flasher/        … WebSerial Flasher / ツール案内
    │
    └── /adx/archive/                         (参照専用・通常導線から分離)
        ├── /adx/archive/8854-formfactor/     … 旧 8854 規格（Prototype CARD v0 参照用）
        └── /adx/archive/8748-formfactor/     … 旧 8748 規格
```

### 3.1 ボード図鑑ページ（`/hardware/*`）の標準テンプレート

開発メモ（`concept.md` や `rev0〜rev4`）をそのまま載せず、外部閲覧者向けに **「1 ボード＝1 ページの清書フォーマット」** で統一します。

1. **ステータスバッジ帯**:
   - `Status`（例: `Alpha / Gerber Verified`, `Prototype v0`, `In Design (rev4)`）
   - `Form Factor`（例: `8858 Official Alpha` / `8854 Legacy`）
   - `Bus Standard`（例: `ADX-BUS-SPEC-v4.2`）
   - ※ `Prototype CARD v0`（8854 準拠）と `Reference CARD v0`（8858 準拠）の過渡期差異もここで正直に明示する。
2. **ヒーロー外観写真 / 3D レンダリング**:
   - 既存アセット（`adx_reference_card_v0_2026-11-23.png`, `adx_prototype_card_v0.png` 等）を活用し、実基板到着後に写真へ差し替え。
3. **1 分サマリー**: このボードは何のためのものか（3 行以内）。
4. **ビジュアルピンアウト図（SVG）**: Arduino.cc 風のカラーピンアウト図（クリックで拡大 / SVG・PDF ダウンロード）。
5. **主要スペック表**: 外形・電源・MCU・I/F・保護回路の要点。
6. **設計データ＆ダウンロード**: Gerber / Netlist / Schematic / STEP（GitHub リポジトリ上の実体へのリンク）。
7. **使用上の注意点 & 関連リンク**: `S00` N.C. 要件や 3.3V ロジック注意喚起、詳細設計ログ（GitHub）へのリンク。

---

## 4. 多言語（i18n）と SSOT 疎結合アーキテクチャ

日英対応と二重管理防止を両立するため、コンテンツを **3 レイヤー** に分けます。

```text
┌──────────────────────────────────────────────────────────────────────┐
│ Layer 1: 言語非依存・共通アセット（翻訳しない / 1 本のみ管理）         │
│  ・SVG ピンアウト図 / 8858 寸法図 / ボード写真                        │
│  ・docs/spec/ADX_PINOUT.md, docs/spec/mcu_mapping/*.md (英語 SSOT)   │
│    → Starlight 自動フォールバックにより /ja/ 側でも 404 なしで閲覧可   │
├──────────────────────────────────────────────────────────────────────┤
│ Layer 2: 既存公式規格（docs/en/, docs/ja/ からビルド時自動同期）       │
│  ・8858_formfactor.md, ADX_pinout.md, adx_formfactor_guidelines.md 等 │
│  ・site/scripts/sync-docs.mjs がビルド直前にフロントマター付与＆取込  │
│  ・取り込み先は .gitignore 対象とし、リポジトリ内の二重管理を完全排除  │
├──────────────────────────────────────────────────────────────────────┤
│ Layer 3: サイト専用の外向け清書ページ（site/src/content/docs/）        │
│  ・トップ LP (index.mdx / ja/index.mdx)                              │
│  ・ボード図鑑 (hardware/*)                                           │
│  ・筐体設計者／回路設計者向けハブページ (enclosure/index, circuit/index)│
│  ・英語を正本とし、日本語版は重要ページから順次追加（未訳は自動フォールバック）│
└──────────────────────────────────────────────────────────────────────┘
```

### Starlight 多言語設定のメリット
- **未翻訳ページが 404 にならない**: `defaultLocale: 'root'`（英語）＋ `ja`（日本語）構成により、日本語版がまだ無いページへ `/ja/...` でアクセスしても自動で英語版＋日本語注記バナーを表示。
- **日本語の全文検索が標準動作**: Starlight 内蔵の **Pagefind** により、外部検索サービスや API キー不要で日本語・英語ともに高精度なローカル検索が機能。
- **サイト全体で言語切替が一貫**: トップ LP（`/` ⇄ `/ja/`）から各リファレンスページまで、ヘッダーの言語切替 1 つで対応ページへシームレスに遷移。

---

## 5. ビジュアルリファレンス（ピンアウト図・寸法図）の設計方針

「Arduino のピンアウト画像」や「筐体設計者向け寸法図」はハードウェアサイトの価値の中核になるため、保守しやすい **SVG ファースト** で整備します。

### 5.1 ADX 20-Pin Bus 共通カラーコード案
すべてのボード図・バス図で配色を統一し、初見でも 1 秒で信号系統を識別できるようにします。

| 信号系統 | 対象ピン | カラー系統（案） |
| :--- | :--- | :--- |
| **Power (`VBUS` 5V)** | Pins 17–20 | Red / Crimson（注意・大電流幹線） |
| **Ground / Shield (`GND`)** | Pins 1, 4, 6, 11, 16 | Dark Gray / Black |
| **System (`S00 / OWD`)** | Pin 2 | Slate + 斜線／警告枠（Card N.C. 必須） |
| **Analog (`P01 / AIN`)** | Pin 3 | Amber / Orange |
| **Clock / System (`P02 / EXTCLK`)** | Pin 5 | Purple / Violet |
| **UART (`P03–P04`)** | Pins 7–8 | Cyan / Teal |
| **I2C (`P05–P06`)** | Pins 9–10 | Emerald / Green |
| **SPI (`P07–P10`)** | Pins 12–15 | Blue / Indigo |

### 5.2 優先作成する図面アセット
1. **`adx-bus-v4.2-pinout.svg`**（Phase 1）：2×10P IDC ヘッダーのトップビュー＋全 20 ピン機能・シールド構造図（全ボード共通）。
2. **`8858-mechanical-zones.svg`**（Phase 1）：`88.0 × 58.0 mm` 外形、M3 穴（`76.0 × 46.0 mm`）＋`Φ10.0 mm` 座面、左右 `3.0 mm` カードガイド、`66.0 mm` I/O Edge、下面 SMD Only ゾーン、`M1–M4` 光学マーカー領域、20P IDC `(44.0, 51.0)` を 1 枚に凝縮した寸法図。
3. **ボード別 Arduino 風ピンアウトシート**（Phase 2）：
   - `core-u-rev4-pinout.svg`（ATtiny1616 ポート名・`ADX_P01`〜`P10`・`USB-PWR`/`USB-COM`・オンボード LED を図示）
   - `prototype-card-v0-pinout.svg`（`U1` / `H1` 2×20P ブレイクアウト対応図）
   - `reference-card-v0-overview.svg`（各計測基準点・光学マーカー `100010`/`100011` 図示）

※ 図面内のラベルは英語・記号で統一し、日英ページで同一 SVG を共有します。

---

## 6. ディレクトリ構成とデザイン方針（`site/`）

### 6.1 想定ディレクトリ構造
```text
site/
├── README.md                     # サイト運用・ローカル開発ガイド（更新）
├── package.json                  # astro, @astrojs/starlight 等
├── astro.config.mjs              # Starlight 設定（locales, sidebar, customCss）
├── scripts/
│   └── sync-docs.mjs             # ../docs/spec/, ../docs/en|ja/ からの SSOT 同期
├── public/
│   └── assets/
│       ├── logos/                # 既存 arle_logo.png, adx_v5_logo.png, furukawakikaku_logo.png
│       └── diagrams/             # ダウンロード可能な SVG / PDF 原本
└── src/
    ├── styles/
    │   └── custom.css            # ARLE.cc ダーク工業テーマ（#090a0d + グリッド + JetBrains Mono）
    ├── assets/
    │   ├── boards/               # ボード写真・レンダリング画像（Astro 画像最適化対象）
    │   └── diagrams/             # 記事埋め込み用 SVG
    └── content/
        └── docs/
            ├── index.mdx         # 英語トップ LP（template: splash）
            ├── hardware/         # ボード図鑑（EN）
            ├── enclosure/        # 筐体設計者ハブ（EN）
            ├── circuit/          # 回路設計者ハブ（EN）
            ├── ja/               # 日本語ページ（未訳は自動で EN にフォールバック）
            │   ├── index.mdx
            │   ├── hardware/
            │   ├── enclosure/
            │   └── circuit/
            └── _synced/          # [gitignore] sync-docs.mjs がビルド時に生成
```

### 6.2 デザイン方針
- 現在の [site/index.html](../../site/index.html) および [draft_lp.html](../../site/src/pages/draft_lp.html) のブランドトーン（**ダーク背景 `#090a0d`、微細な設計グリッド、`Inter` ＋ `JetBrains Mono`、抑制の効いたグリーン／シアンアクセント**）を `custom.css` で Starlight テーマに反映する。
- Starlight 標準のライト／ダーク切替は残しつつ、デフォルト（またはブランド表現）として工業用計測器・データシートのような清潔でコントラストの高い見た目を保つ。

---

## 7. 段階的開発計画（マイルストーン）

まずは **Milestone 1（Starlight 化＋Cloudflare Pages 疎通の最小縦貫通セット）** でインフラと多言語・検索の動作を本番確認してから、コンテンツを段階的に積み上げます。

| マイルストーン | 目的・スコープ | 完了条件（DoD） |
| :--- | :--- | :--- |
| **M1: Starlight 化 ＆ Cloudflare 開通（最小セット）** | ・`site/` に Astro ＋ Starlight を初期化<br>・現行 [site/index.html](../../site/index.html) のダーク世界観を継承したトップ LP（EN `/`・JA `/ja/`）<br>・疎通確認用の最小ページ（Overview EN/JA＋未訳フォールバック確認用 1 ページ）<br>・ローカルビルド確認後、`main` push ＆ Cloudflare Pages ビルド設定切替 | `https://arle.cc/` と `/ja/` が Cloudflare Pages の自動ビルドで表示され、ロゴ・日英切替・Pagefind 検索が正常動作する |
| **M2: 公式規格の SSOT 自動同期** | ・`site/scripts/sync-docs.mjs` 実装（`docs/spec/`, `docs/en/`, `docs/ja/` をビルド時同期）<br>・筐体（`8858`）＆ 回路（`Pinout v4.2`, `MCU Mapping`, `Guidelines`）のハブページ整備<br>・旧規格（`8854`, `8748`）を Archive 枠へ分離 | `docs/` の Markdown を修正して push するだけで `arle.cc` が自動更新され、二重管理が発生しない |
| **M3: ボード図鑑 ＆ SVG ピンアウト図** | ・ボード図鑑 3 ページ（`CORE-U` / `Reference CARD v0` / `Prototype CARD v0`）<br>・共通 SVG 図面 2 点（`adx-bus-v4.2-pinout.svg`, `8858-mechanical-zones.svg`）<br>・実基板到着後に実機写真・ボード別ピンアウト図・STEP / DXF ダウンロードを順次拡充 | 基板を受け取った人・筐体や CARD を設計する人がサイトだけで仕様と図面を把握できる |
| **M4: ソフトウェア ＆ 作例** | ・Arduino IDE (`megaTinyCore`) セットアップ<br>・`ADX_<BOARD>.h` コードリファレンス<br>・Desktop Andon 等の作例（Tutorials / Projects）および WebSerial Flasher 導線 | 「買って・書いて・箱に入れる」までの一気通貫ドキュメントが完成する |

---

## 8. ビルド方式の決定と Cloudflare Pages 切替手順

### 8.1 ビルド方式：Cloudflare Pages 側ビルドを採用
ローカルビルドで生成 HTML を Git コミットする案と比較した結果、Starlight はビルドごとにハッシュ付きアセットや Pagefind 検索インデックス（バイナリ）を生成するため、ローカルビルドにしても全体の複雑性は下がらずリポジトリ履歴にノイズを引き込むと判断。**ソースのみを Git 管理し、Cloudflare Pages 側で自動ビルドする方式** を正式採用します。

### 8.2 M1 での安全な切替手順
1. ローカル（Node `v20`）で `site/` に Starlight を構築し、`npm run build && npm run preview` でトップ LP・日英切替・検索・画像表示を事前検証する。
2. 確認完了後、既存の `site/index.html` を Starlight 構成へ置き換えて `main` へ push する。
3. push と同時に Cloudflare Pages ダッシュボード（`arle.cc` → **Settings → Build**）で以下を設定する：
   - **Root directory**: `site`（そのまま）
   - **Build command**: `npm run build`
   - **Build output directory**: `dist`
   - **Environment variables**: `NODE_VERSION = 20`（ローカル環境と揃える）
   - **Build watch paths（Include paths）**: `site/*`, `docs/*`
4. Cloudflare Pages の初回ビルド成功を確認し、`https://arle.cc/` および `https://arle.cc/ja/` の表示・検索を実確認する。

### 8.3 Build watch paths（`site/*`, `docs/*`）を入れる理由
Cloudflare Pages の公式仕様では、`Build watch paths` の `*` はサブディレクトリ（`/`）も含めて再帰マッチします。
- `docs/*` を最初から含めておくことで、M2 以降で `docs/spec/` だけを更新した push でも **ビルド抜けが発生しません**。
- 逆に `memo/`・`hardware/`・`firmware/`・`podcast/` の更新では無駄な再ビルドが走らず、Cloudflare Pages の月間ビルド枠とデプロイログを綺麗に保てます。

---

## 9. M1 着手前の確認ポイント

M1（最小セット）の実装に入る前に、以下だけ軽く確認しておくと後戻りがありません：

1. **URL の階層に `/adx/` を挟むか？**（例：`arle.cc/hardware/...` vs `arle.cc/adx/hardware/...`）
   - M1 では Overview ページ 1〜2 枚だけですが、トップ LP からのリンク先を最初から決めておくと M2 以降で URL が変わりません。
2. **M1 の実装に進めてよいか？**
   - OK いただければ、ローカルで `site/` の Starlight 最小セットを構築・ビルド確認するところまで進めます（`git push` と Cloudflare Pages の設定切替は確認いただいたタイミングで実施）。
