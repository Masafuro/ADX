<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CERN-OHL-P-2.0
-->

# ADX ハードウェアリポジトリ (Hardware Repository)

本ディレクトリは、ADXエコシステムにおけるハードウェア設計データ（回路図、PCBアートワーク、BOM、製造データ等）を管理・公開するリポジトリです。

---

## 1. ハードウェアラインナップ

ADXのハードウェアは、**8748 Form Factor**（`87.0 mm × 48.0 mm`）および共通の **20ピン IDC 拡張バス（ADX Pinout v2）** を共有する「Coreシリーズ（MCUボード）」と「CARDシリーズ（機能拡張基板）」で構成されます。

### 1.1 Core シリーズ (メインMCUボード)

| ボード名 | グレード | 絶縁構造 | 状態・概要 | リンク |
| :--- | :--- | :--- | :--- | :--- |
| **ADX CORE-I** | **Production** | **両絶縁** (電源・通信) | ADXの本命標準機。外部機器（PLC・PC・異電源系）と安全に接続可能。 | [設計検討中](CORE-I/CORE-S_strategy_rethink.md) |
| **ADX CORE-S-P** | **Professional** | **非絶縁** (共通GND) | コスト極限・超低価格製造（$5〜7/台）を追求したプロ専用機。自己責任DIY製造前提。 | [詳細・レビュー](CORE-S-P/README.md) |
| **ADX Core-D** | **Development** | **非絶縁** | LN-485（LIN-based RS-485）ブートローダおよびファームウェア開発専用ボード。 | [仕様書](Core-D/proposal.md) |

### 1.2 CARD シリーズ (拡張アドオン基板)

| ボード名 | 概要 | リンク |
| :--- | :--- | :--- |
| **ADX Prototyping CARD** | 下列20P ADXバス引き出し ＋ 上列20P ユニバーサル試作エリアを備えたプロトタイピング拡張基板。 | [仕様書](CARD/Prototyping/proposal.md) |

### 1.3 フォームファクタ & 規格仕様

- **8748 フォームファクタ定義**: [`Formfactor/`](Formfactor/) / [仕様書](../docs/ja/8748_formfactor.md)
- **ADX ピンアサイン (v2)**: [仕様書](../docs/ja/ADX_pinout.md)

---

## 2. 命名規則・グレード分類（Production vs `-P`）

ADXのハードウェアは、安全性および提供形態に応じて以下の2つのグレードに明確に分類されています。

```text
ADX [FAMILY]-[TYPE][-SUFFIX]
```

1. **Production Grade（通常版・サフィックスなし）**:
   - 例: `ADX CORE-I`, `ADX CARD-RELAY`
   - **要件**: 電源・通信の完全絶縁またはフェイルセーフ保護を備え、誤配線時にも接続先の外部機器（PC、PLC等）を連鎖破壊しない安全設計。
   - **提供形態**: 完成品販売および公式サポート対象。
2. **Professional / DIY Grade（`-P` 版）**:
   - 例: `ADX CORE-S-P`, `ADX CARD-RELAY-P`
   - **要件**: コスト削減や超小型化を優先した共通GND・非絶縁構造。誤配線時の連鎖破壊リスクを自ら理解・管理できるエンジニア向け。
   - **提供形態**: 設計データ（BOM、Gerber、CPL等）の完全公開による**自己責任DIY製造（PCBA発注）**前提。

※ 詳細な分類判定基準については、[ハードウェア命名規則・グレード分類基準](../docs/ja/hardware_naming_rules.md) を参照してください。

---

## 3. ライセンス

本ディレクトリ内のすべてのハードウェア設計データは、**CERN-OHL-P-v2（CERN Open Hardware Licence Version 2 - Strongly Reciprocal）** のもとで公開されています。
詳細は [LICENSE.md](../LICENSE.md) および [LICENSES/CERN-OHL-P-2.0.txt](../LICENSES/CERN-OHL-P-2.0.txt) を参照してください。
