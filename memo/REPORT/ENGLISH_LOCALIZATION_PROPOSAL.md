# ADX リポジトリにおける英語表記・多言語展開の検討方針

本ドキュメントは、ADX プロジェクトのグローバル展開（Crowd Supply、オープンソースハードウェアコミュニティへの発信）を見据えた、リポジトリの英語表記方針・ドキュメント構造・用語定義の検討まとめです。

---

## 1. リポジトリのドキュメント構造方針（3つの選択肢）

海外展開を視野に入れた場合、以下のいずれの構成をとるかを検討します。

### オプション A: 英語を主軸（デフォルト）とし、日本語版を併設する【推奨】
* **構成**:
  * `README.md`: **英語**（グローバル向けメインドキュメント）
  * `README_JA.md`: **日本語**（国内開発者・詳細メモ用）
  * `memo/` または `docs/`: 規格仕様書を英語版ベースにし、日本語版（`_ja.md`）を併置
* **メリット**:
  * GitHub の標準的な OSS プロジェクトとして海外コントリビューターや Crowd Supply の閲覧者が違和感なくアクセス可能。
  * リポジトリのトップページが即座に英語で提示されるため、国際的な認知獲得に有利。
* **運用**: 先頭に `[日本語版はこちら / Japanese version](README_JA.md)` のリンクを配置。

### オプション B: 日本語をデフォルトとし、英語版（README_EN.md）を追加する
* **構成**:
  * `README.md`: **日本語**（現在の状態を維持）
  * `README_EN.md`: **英語**
* **メリット**:
  * 国内の開発・思考プロセスをそのまま主軸に置ける。
  * 既存のリンク構造を崩さずに移行可能。
* **デメリット**:
  * 海外ユーザーがアクセスした際、ワンクッション（リンククリック）が必要。

### オプション C: `docs/` フォルダによる多言語分離
* **構成**:
  * `README.md`: 概要（英語）＋ 各言語版へのリンク
  * `docs/en/` : 英語ドキュメント一式
  * `docs/ja/` : 日本語ドキュメント一式
* **メリット**:
  * ドキュメント数が増加した際に構造が最もクリーン。
* **デメリット**:
  * 小〜中規模の現フェーズではやや過剰な階層構造になる可能性。

---

## 2. コア用語・技術表現の英語定義 (Terminology Standard)

技術的な正確性とプロフェッショナルな印象を両立するため、以下の統一用語を採用することを提案します。

| 日本語 | 推奨英語表記 | 補足・ニュアンス |
| :--- | :--- | :--- |
| **ADX プロジェクト** | **ADX: Advanced Devices eXtended** | 正式名称 |
| **キャッチフレーズ** | **A Modular Hardware Standard for Industrial Control & Embedded Systems** | 大げさにならず、産業向けモジュラー規格であることを明確化 |
| **8748 フォームファクタ** | **8748 Form Factor Specification** | - |
| **エレメカ協調設計** | **Electromechanical Co-design** | CAD間の寸法整合性を指す国際的な業界用語 |
| **基板CADと筐体CADの寸法整合** | **Metric & Mil Coordinate Alignment** | - |
| **理論丸め誤差** | **Theoretical Rounding Error / Mil-to-Metric Conversion Tolerance** | 15μm未満の精度説明 |
| **嵌合性 / クリアランス** | **Mechanical Fit & Clearance** | - |
| **ADX ピンアサイン** | **ADX Pinout Specification (20-Pin Unified Interface)** | - |
| **干渉低減 / アイソレーション** | **Crosstalk Mitigation / Noise Isolation Buffer** | 静的ピン（RE/DE）やGNDシールドの役割 |
| **LN-485** | **LN-485 (LIN-based Physical Layer RS-485 Protocol)** | 半二重差動通信の規格説明 |
| **ADX Core-D** | **ADX Core-D (LN-485 & Bootloader Development Board)** | - |
| **ADX Prototyping CARD** | **ADX Prototyping Card (Stackable Add-on Card)** | - |

---

## 3. 各ドキュメントにおける英語化の要点

### 3.1 `README.md` (Top Page)
* **Header / Pitch**:
  * 1〜2文で「何のための規格・基板なのか」を定義（オープンソースの柔軟性＋産業グレードの信頼性）。
* **Key Specifications**:
  * `8748 Form Factor`: 87.0 × 48.0 mm (77.0 × 38.0 mm pitch, M3 mounting) を箇条書きとテーブルで明示。
  * `ADX Pinout`: 20-Pin CN2 のピンアサインテーブルを英語表記（Net, GPIO, Primary/Special Functions, Notes）。
* **Hardware Lineup**:
  * Core-D, Prototyping Card の主要スペック（MCU, Peripherals, I/O, Protection）を簡潔に紹介。
* **Quick Links**:
  * データシート、製造データ（Gerber/BOM）、関連サイトへのリンク。

### 3.2 `8748_formfactor.md`
* **Mechanical Specification**:
  * Board outline (`87.0 mm x 48.0 mm`), Mounting pitch (`77.0 mm x 38.0 mm`), Hole margin (`5.0 mm`), Hole diameter (`Φ3.3 mm`).
* **Electrical / PCB Specification**:
  * Edge.Cuts (`3425 mil x 1890 mil`), Hole pitch (`3031 mil x 1496 mil`), Margin (`197 mil`), Drill (`130 mil`).
* **Tolerance & Precision Analysis**:
  * 理論誤差比較テーブル（mm名目値 vs mil実効値、誤差<15μm、M3ネジの公差吸収など）を英文テーブル化。

### 3.3 `ADX_pinout.md`
* **Design Philosophy**:
  * クロック信号（EXTCLK）および通信線（RxD, SCK）へのGNDシールド配置。
  * 静的制御ピン（RE/DE）をアナログ／I2C間の緩衝ライン（Buffer Line）として配置する設計意図。
* **Pin Table & Footnotes**:
  * 英語の機能説明テーブルと、Master/Slave時の挙動に関する注記。

---

## 4. 今後の進め方（提案）

1. **方針の確定**:
   * ドキュメント配置（オプション A の `README.md (EN)` + `README_JA.md (JA)` を採用するか等）の決定。
2. **英語ドラフトの作成**:
   * `memo/REPORT/` 配下に英語ドラフト（`README_EN_PROPOSAL.md` 等）を作成し、表現や構成をレビュー。
3. **実ファイルへの適用**:
   * レビュー完了後、正式に各ファイルへ反映。
