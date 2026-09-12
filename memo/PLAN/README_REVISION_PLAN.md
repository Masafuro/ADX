<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX README 改訂計画書 (README Revision Plan)

## 1. 背景と改訂の目的

### 1.1 プロトタイピングボードにおける筐体適合性の課題
Arduino Uno や Raspberry Pi 等の標準的な開発ボードは、卓上でのプロトタイピングにおいて広く普及しています。一方で、試作した回路を実際の筐体に収め、現場や設備へ設置（据付）する段階では、以下のような物理的・構造的制約が生じやすい側面があります。

1. **締結マージンとワッシャーのクリアランス**:
   - 取付穴周辺に部品や配線パターンが近接しており、標準的なワッシャーや座金が干渉しやすい。
   - M3等の標準ネジで適切なトルクをかけて確実に固定する際、基板への負荷や破損リスクに配慮が必要となる。
2. **多方向コネクタによる開口加工の複雑化**:
   - 基板の複数辺（前後左右）からUSB、電源ジャック、ピンソケット等が突出しているため、筐体のパネル加工や防水・防塵パッキンの設計が煩雑になりやすい。
3. **剛結合スタックによる公差吸収の難しさ**:
   - シールド等の拡張基板との接続がピンヘッダの直接勘合（剛結合）を前提としているため、部品の高さばらつきや基板の反り、筐体側の寸法公差を逃がしにくい。
   - その結果、家庭用3Dプリンタや簡易加工など、製造公差の大きい手段で筐体を作製する際に嵌合の調整が難しくなる。

### 1.2 記述の見直しとポジショニングの再整理
- **現行記述の課題**: 先行して開発された **ADX Core-D** の機能（LN-485通信、耐ノイズ性能）を基に記述が構成されていたため、プロジェクト全体が「産業用途（Industrial）専用」に限定されているような印象を与えていました。
- **改訂の目的**: 汎用的な開発用途を想定した **ADX CORE-U** の展開に伴い、ADXの本質である**「8748フォームファクタと共通バスを基盤とし、筐体への組み込みやすさと現場据付性を高めたハードウェアプラットフォーム」**として、ドキュメント全体の表現を適切に位置づけ直します。

---

## 2. 8748 Form Factor の設計アプローチ

ADXは、Arduino IDE等による既存の開発利便性を維持しながら、筐体設計や現場設置に伴う構造的課題を軽減する仕様を定義しています。

| 設計項目 | 一般的なプロトタイピング基板の傾向 | ADX (8748 Form Factor) のアプローチ |
| :--- | :--- | :--- |
| **ネジ締結** | 穴周囲のクリアランスが狭く、ワッシャー併用が難しい | **Φ3.3 mm穴（M3ネジ対応）と周囲5.0 mmのマージン**を確保。ワッシャーを用いた確実な締結が可能。 |
| **外部I/Fの配置** | 複数辺にコネクタが分散配置され、開口部が多い | **一貫したI/F辺の定義**。端子台や外部接続の方向を揃え、筐体側の加工面を集約。 |
| **基板間接続** | ピンヘッダによる直接スタック（剛結合） | **IDCフラットケーブルによる接続（柔結合）**。配置の自由度を確保し、寸法公差を吸収可能。 |
| **筐体適合性** | 専用設計のケースを前提とすることが多い | **家庭用3Dプリンタ**での出力、市販の**防水ボックス・電工ボックス**への取付、専用ラックへの**スロットイン**に対応。 |

---

## 3. ハードウェア構成の整理

共通規格である **8748 Form Factor**（外形・取付穴）と **ADX Pinout**（20ピン共通拡張バス）を基点として、役割ごとに基板を展開します。

```text
       ┌─────────────────────────────────────────────────────────────┐
       │                   8748 Form Factor (87 × 48 mm)             │
       │           M3ネジ締結対応 / 一貫したI/F辺 / ボックス取付適合        │
       └──────────────────────────────┬──────────────────────────────┘
                                      │
               ┌──────────────────────┴──────────────────────┐
               ▼                                             ▼
     【Core シリーズ (MCU基板)】                     【CARD シリーズ (拡張基板)】
  ┌─────────────────────────┐                   ┌─────────────────────────┐
  │      ADX CORE-U         │                   │  ADX Prototyping CARD   │
  │ ・汎用・標準プロトタイプ│                   │ ・20P バス信号引き出し  │
  │ ・Arduino互換の扱いやすさ│                  │ ・試作・ユニバーサル領域│
  └────────────┬────────────┘                   └────────────┬────────────┘
  ┌────────────┴────────────┐                                │
  │       ADX Core-D        │                                │
  │ ・LN-485 (RS-485/LIN)   │                                │
  │ ・差動通信・高ノイズ環境向け│                              │
  └────────────┬────────────┘                                │
               │                                             │
               └─────────── [ IDC リボンケーブル (柔結合) ] ───┘
```

1. **Core シリーズ (メインMCU基板)**:
   - **ADX CORE-U**: Unoに近い感覚で手軽に扱え、そのまま実機筐体に組み込める汎用スタンダード基板。
   - **ADX Core-D**: LN-485（RS-485/LIN）トランシーバを搭載し、産業機器やフィールドネットワークとの接続に適した基板。
2. **CARD シリーズ (機能拡張基板)**:
   - **ADX Prototyping CARD**: 20ピンADXバスから信号を引き出し、カスタム回路を実装できる拡張基板。
   - IDCフラットケーブル接続により、筐体内部の空間に応じてスタック配置や並列配置を選択可能。

---

## 4. 新 README.md の構成案

過度な修飾表現を避け、客観的な技術情報と設計意図が明確に伝わる構成とします。

```markdown
# ADX (Advanced Devices eXtended)
[ English | 日本語 ]

ADX（Advanced Devices eXtended）は、筐体適合性と現場据付性を重視したオープンソースのハードウェア規格およびモジュラー制御プラットフォームです。
一般的なプロトタイピング環境の扱いやすさを維持しながら、確実なネジ締結、配線方向の集約、フレキシブルな基板間接続によって、実機への組み込みを容易にします。

---

## 1. 概要 (Overview)
- プロジェクトの目的と背景
- プロトタイピングから筐体実装への移行における課題と解決策

## 2. 8748 Form Factor の特徴 (Key Characteristics)
- M3ネジおよび標準ワッシャーに対応した締結マージン
- 配線引き出し面を集約する一貫したI/F辺
- IDCリボンケーブルを用いた柔結合による公差吸収とデイジーチェーン接続
- 市販ボックス（防水・電工ボックス）、3Dプリントケース、スロットイン構造への対応

## 3. ハードウェアラインナップ (Hardware Lineup)
- **Core シリーズ (MCU基板)**:
  - ADX CORE-U: 汎用プロトタイピング向けスタンダードモデル
  - ADX Core-D: LN-485通信機能を備えた高信頼性・差動通信モデル
- **CARD シリーズ (拡張基板)**:
  - ADX Prototyping CARD: 20ピンバス接続のユニバーサル拡張基板

## 4. コア仕様 (Specifications)
- 8748 Form Factor 外形図および寸法要約
- ADX Pinout（20ピン共通拡張コネクタ）のピンアサイン要約
- 各詳細仕様書（docs/配下）へのリンク

## 5. ドキュメント構成 (Documentation)
- 日本語 / 英語ドキュメントの案内
- リポジトリディレクトリマップ

## 6. ライセンス (License)
- 仕様書 (CC BY 4.0)、ハードウェア設計 (CERN-OHL-P-v2)、ファームウェア (MIT)
```

---

## 5. ドキュメント改訂の文面対比（例）

### 5.1 リード文の推敲例

- **現行文 (Root README.md / docs/en)**:
  > ADX (Advanced Devices eXtended) is an open-source hardware standard and modular control platform designed to provide industrial-grade reliability, noise immunity, and electromechanical precision with the simplicity of open-source embedded development.

- **改訂案 (To-Be)**:
  > **ADX (Advanced Devices eXtended)** is an open-source hardware standard and modular platform focused on electromechanical enclosure fit and field installation.
  > 
  > While conventional prototyping boards offer great accessibility on the benchtop, mounting them into real-world enclosures often presents mechanical challenges, such as tight screw clearances, multi-directional cabling, and rigid stacking constraints.
  > 
  > ADX addresses these integration challenges through the **8748 Form Factor** and a flexible IDC-based expansion bus—providing reliable M3 screw mounting, a unified wiring interface edge, and tolerance-friendly packaging compatible with 3D-printed enclosures, industrial electrical boxes, and waterproof casings, all while maintaining familiar open-source embedded workflows.

---

## 6. 作業ロードマップ

1. **Step 1: 計画書の確定** (`memo/PLAN/README_REVISION_PLAN.md`) ★現在地
2. **Step 2: ドラフト原稿の作成** (`memo/REPORT/README_PROPOSAL_V2.md`)
   - 日本語版および英語版の README ドラフトを作成し、内容を確認。
3. **Step 3: 正式ドキュメントへの反映**
   - ルート `README.md`
   - `docs/ja/README.md`
   - `docs/en/README.md`
   - `Project_Snapshot.md`
