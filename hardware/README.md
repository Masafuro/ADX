<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CERN-OHL-P-2.0
-->

# ADX ハードウェアリポジトリ (Hardware Repository)

本ディレクトリは、ADXエコシステムにおけるハードウェア設計データ（回路図、PCBアートワーク、BOM、製造データ等）を管理・公開するリポジトリです。

---

## 1. ハードウェアラインナップ

ADXのハードウェアは、**8748 Form Factor**（`87.0 mm × 48.0 mm`）および共通の **20ピン IDC 拡張バス（ADX Pinout v4.0）** を共有する「Coreシリーズ（MCUボード）」と「CARDシリーズ（機能拡張基板）」で構成されます。

### 1.1 Core シリーズ (メインMCUボード)

| ボード名 | グレード | 保護・通信構造 | 状態・概要 | リンク |
| :--- | :--- | :--- | :--- | :--- |
| **ADX CORE-CX5** | **Production / Flagship** | **デュアル Type-C（通信/電源分離）** | **次世代の標準 USB Core ボード。** WCH CH32X035（32-bit RISC-V 48MHz）、USB-PHY 内蔵、UF2 ドラッグ＆ドロップ書き換え、低価格・高性能。 | [構想書](CORE-CX5/concept.md) / [Mapping 仕様](../docs/spec/mcu_mapping/ch32x035.md) |
| **ADX CORE-A5** | **Production / Active** | **2500V 絶縁RS-485 / VBUS幹線受電** | **高耐環境・高信頼エッジ端末。** -40℃〜+105℃耐熱部品、3.3VネイティブGPIO、前面5VIN端子台、CCL自律リセット。 | [構想書](CORE-A5/concept.md) / [仕様検討書](CORE-A5/spec_study.md) |
| **ADX CORE-U** | **Archive** | **PC保護回路** | 旧USB構想ボード。ATtiny1616（CORE-CX5へ発展継承）。 | [検討ノート](archive/CORE-U/concept.md) |
| **ADX CORE-A** | **Archive** | **完全絶縁RS-485 / 5V給電** | 旧現場向けモデル。-40℃〜+105℃耐熱（CORE-A5へ発展継承）。 | [旧構想書](archive/CORE-A/concept.md) |
| **ADX CORE-S-P** | **Professional** | **非絶縁** (共通GND) | 低価格製造を追求したプロ専用機。自己責任DIY製造前提。 | [詳細・レビュー](archive/CORE-S/README.md) |
| **ADX Core-D** | **Development** | **非絶縁** | LN-485（LIN-based RS-485）ブートローダおよびファームウェア開発専用ボード。 | [仕様書](archive/CORE-D/proposal.md) |

### 1.2 CARD シリーズ (拡張アドオン基板)

| ボード名 | 概要 | リンク |
| :--- | :--- | :--- |
| **ADX Prototyping CARD** | 下列20P ADXバス引き出し ＋ 上列20P ユニバーサル試作エリアを備えたプロトタイピング拡張基板。 | [仕様書](CARD/Prototyping/proposal.md) |

### 1.3 フォームファクタ & 規格仕様

- **8748 フォームファクタ定義**: [`Formfactor/`](Formfactor/) / [仕様書](../docs/ja/8748_formfactor.md)
- **ADX 20ピンバス規格 (SSOT)**: [普遍的バス規格書 (英語・アルファベット)](../docs/spec/ADX_PINOUT.md)（Standard: `ADX-BUS-SPEC-v4.0`）
- **ADX ピン配置 設計ガイド**: [日本語仕様解説](../docs/ja/ADX_pinout.md) / [English Architecture Guide](../docs/en/ADX_pinout.md)

---

## 2. 命名規則・グレード分類（Production vs `-P`）

ADXのハードウェアは、安全性および提供形態に応じて以下の2つのグレードに明確に分類されています。

```text
ADX [FAMILY]-[TYPE][-SUFFIX]
```

1. **Production Grade（通常版・サフィックスなし）**:
   - 例: `ADX CORE-CX5`, `ADX CORE-A5`, `ADX CARD-RELAY`
   - **要件**: 電源・通信のガルバニック絶縁またはフェイルセーフ保護（逆流防止SBD、過電流PTC等）を備え、誤配線時にも接続先の外部機器（PC、PLC等）の破損リスクを低減する安全設計。
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
