<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-S 部品データシート・カタログ (Datasheet Catalog)

本ディレクトリは、ADX CORE-S ネットリスト（[`Netlist_Schematic1_2026-09-16.enet`](../../Netlist_Schematic1_2026-09-16.enet)）に記載された全コンポーネントのデータシート PDF を自動収集・整理したリポジトリです。

---

## 1. 収集状況サマリー

- **収集完了ファイル数**: 30 ファイル（全コンポーネント 100% カバー）
- **総ファイルサイズ**: 約 77 MB
- **格納先ディレクトリ**: [`hardware/CORE-S/review/datasheets/`](./)

---

## 2. 主要 IC・重要部品 データシート精査速報 (Quick Insights)

ダウンロードしたデータシートの本文および電気的特性テーブルを精査した初期結果です。

### ① 降圧 DC-DC コンバータ: `TPS5430DDAR` (TI / LCSC: `C9864`)
- **PDF**: [`TPS5430DDAR_C9864.pdf`](./TPS5430DDAR_C9864.pdf)
- **ENA ピン（Pin 5）の未接続について**:
  - データシート（p.3, p.9, p.25）より、**内部に 1.5MΩ のプルアップ抵抗を内蔵**しており、「ENA 端子はオープン（Floating）のままで自動起動動作が可能」と明記されています。未接続（NC）で動作上の問題はありません。
- **位相補償ループと出力コンデンサの整合性**:
  - 本 IC は内部固定位相補償（Internal Compensation）を採用しています。
  - データシート（p.14）の 5V 出力リファレンス設計例では、**$L=15\,\mu\mathrm{H}$ に対し $C_{OUT}=220\,\mu\mathrm{F}$（最大許容 ESR $\le 40\,\mathrm{m}\Omega$）** が指定されています。
  - 回路で選定されている `1-C5`（創慧電子 220μF 16V 固体電解）は **ESR = 24mΩ** であり、TI の安定動作推奨要件を完全にクリアしています。

### ② ロードスイッチ: `TPS22965DSGR` (TI / LCSC: `C122837`)
- **PDF**: [`TPS22965DSGR_C122837.pdf`](./TPS22965DSGR_C122837.pdf)
- **$V_{IN}$ / $V_{BIAS}$ の関係**:
  - データシート（p.16）より、最小オン抵抗（$R_{ON}$）を維持するためには $V_{IN} \le V_{BIAS}$ が要求されます。
  - 回路では $V_{IN} = 5\mathrm{V}$, $V_{BIAS} = 5\mathrm{V}$（同電位）となっており、定格・推奨条件を満たしています。
- **CT ピン（Pin 6）未接続時の突入電流リスク**:
  - データシート（p.16-18）より、CT ピンがオープンの場合、ソフトスタート時間が最短（$t_R \approx 32\,\mu\mathrm{s}$）となります。
  - VDD 後段容量（約 10μF ＋ IDC 接続カード容量）に対して $I_{inrush} = C_L \times \frac{dV}{dt}$ の急峻な電流が流れ、上流 `+5V` バスが一瞬ドロップする懸念があります。
  - データシート推奨（p.18）に従い、CT 端子に数十〜数百 pF（例: 220pF で $t_R \approx 180\,\mu\mathrm{s}$）のコンデンサを追加する設計変更が強く推奨されます。

### ③ 出力電解コンデンサ: `SS221M160E7R7R4Z00ZZ` (創慧電子 / LCSC: `C25503905`)
- **PDF**: [`SS221M160E7R7R4Z00ZZ_C25503905.pdf`](./SS221M160E7R7R4Z00ZZ_C25503905.pdf)
- **構造**: 導電性高分子固体アルミ電解コンデンサ（Solid Conductive Polymer SMD）
- **主要定格**:
  - 定格電圧: 16 V / 静電容量: 220 μF (±20%)
  - **等価直列抵抗 (ESR @ 100kHz): 24 mΩ**
  - **許容リプル電流 (105℃, 100kHz): 2,900 mArms (2.9 A)**
  - 損失角正接 ($\tan\delta$): 0.1
- **評価**: TPS5430 のスイッチングリップル（約 143mArms）に対し、許容リプル電流 2.9A は約 20 倍のマージンがあり、発熱・寿命面で極めて優秀です。

### ④ RS-485 トランシーバ: `SP485EEN-L/TR` (MaxLinear / LCSC: `C6855`)
- **PDF**: [`SP485EEN-L_TR_C6855.pdf`](./SP485EEN-L_TR_C6855.pdf)
- **ESD / 保護定格**:
  - 内部保護として IEC61000-4-2 接触放電保護が追加されていますが、過電圧サージ（Surge / Lightning）や高エネルギー EFT に対する定格は記載されていません。
  - FA・長距離バス用途では、外部 TVS（Core-D で採用された `PSM712` 等）の追加が望ましいです。

### ⑤ BMC: `ATtiny412-SSNR` (Microchip / LCSC: `C1337190`)
- **PDF**: [`ATTINY412-SSNR_C1337190.pdf`](./ATTINY412-SSNR_C1337190.pdf)
- **ピン構成**: 8 ピン SOIC
  - Pin 1: VDD (+5V)
  - Pin 2: PA6 (`VDD_SW` 制御出力)
  - Pin 3: PA7 (NC / 未使用)
  - Pin 4: PA1 (`BOOT_REQ` 制御出力)
  - Pin 5: PA2 (`BMC_RO` $\to$ **要修正：他素子への未接続**)
  - Pin 6: PA0/RESET/UPDI (`BMC_UPDI` テストピン H2 へ)
  - Pin 7: PA3/EXTCLK (NC / 未使用)
  - Pin 8: GND
- **課題**: Pin 5 (PA2) から出ている 1kΩ 抵抗 `5-R1` の先（ネット `BMC_RO`）が孤立しているため、RS-485 の RO（`PA2/R`）等への配線接続が必要です。

---

## 3. 全部品データシート一覧 (Category-wise Index)

### 3.1 集積回路 (IC / MCU / Power)

| リファレンス | 部品型番 (Manufacturer Part) | LCSC 品番 | メーカー | 概要 / 機能 | ローカル PDF リンク |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `3-MCU` | **ATTINY1616-MNR** | C507118 | Microchip | メイン MCU (QFN-20, Flash 16KB, 20MHz) | [`ATTINY1616-MNR_C507118.pdf`](./ATTINY1616-MNR_C507118.pdf) |
| `5-BMC` | **ATTINY412-SSNR** | C1337190 | Microchip | 基板管理 BMC (SOIC-8, Flash 4KB) | [`ATTINY412-SSNR_C1337190.pdf`](./ATTINY412-SSNR_C1337190.pdf) |
| `1-DCDC` | **TPS5430DDAR** | C9864 | TI | 3A 降圧 DC-DC コンバータ (SOIC-8 EP) | [`TPS5430DDAR_C9864.pdf`](./TPS5430DDAR_C9864.pdf) |
| `2-P_SW` | **TPS22965DSGR** | C122837 | TI | 4A 超低 RON ロードスイッチ (WSON-8) | [`TPS22965DSGR_C122837.pdf`](./TPS22965DSGR_C122837.pdf) |
| `4-RS485` | **SP485EEN-L/TR** | C6855 | MaxLinear | 半二重 RS-485 トランシーバ (SOIC-8) | [`SP485EEN-L_TR_C6855.pdf`](./SP485EEN-L_TR_C6855.pdf) |

### 3.2 ディスクリート・保護素子 (Diodes, TVS, Beads, Inductors)

| リファレンス | 部品型番 | LCSC 品番 | メーカー | 概要 / 定格 | ローカル PDF リンク |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0-D4` | **SMAJ20A** | C353466 | 晶导微电子 | TVS ダイオード (Vrwm=20V, 400W, SMA) | [`SMAJ20A_C353466.pdf`](./SMAJ20A_C353466.pdf) |
| `0-D1`, `1-D2` | **SS54** | C22452 | 辰达半导体 | ショットキーダイオード (40V 5A, SMA) | [`SS54_C22452.pdf`](./SS54_C22452.pdf) |
| `1-L1` | **HCMA-0630-150-M-K** | C54431704 | 佑驰 (YOUCHI) | パワーインダクタ (15μH 3A, 7.3x6.6mm) | [`HCMA-0630-150-M-K_C54431704.pdf`](./HCMA-0630-150-M-K_C54431704.pdf) |
| `0-L2` | **HCB1608KF-600T40** | C356980 | 台庆 (TAI-TECH) | 電源用フェライトビーズ (60Ω@100MHz 0603) | [`HCB1608KF-600T40_C356980.pdf`](./HCB1608KF-600T40_C356980.pdf) |
| `LED-5V`, `LED-VDD` | **KT-0603W** | C2290 | KENTO | 白色 SMD LED (0603) | [`KT-0603W_C2290.pdf`](./KT-0603W_C2290.pdf) |
| `LED-PB4` | **KT-0603R** | C2286 | KENTO | 赤色 SMD LED (0603) | [`KT-0603R_C2286.pdf`](./KT-0603R_C2286.pdf) |

### 3.3 コンデンサ (Capacitors)

| リファレンス | 部品型番 | LCSC 品番 | メーカー | 概要 / 定格 | ローカル PDF リンク |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `1-C5` | **SS221M160E7R7R4Z00ZZ** | C25503905 | 创慧电子 | 導電性高分子アルミ電解 (220μF 16V, ESR=24mΩ) | [`SS221M160E7R7R4Z00ZZ_C25503905.pdf`](./SS221M160E7R7R4Z00ZZ_C25503905.pdf) |
| `1-C1` | **CL31A106KBHNNNE** | C13585 | 三星 (Samsung) | MLCC (10μF 1206 50V X7R) | [`CL31A106KBHNNNE_C13585.pdf`](./CL31A106KBHNNNE_C13585.pdf) |
| `0-C7` | **CL21A106KAYNNNE** | C15850 | 三星 (Samsung) | MLCC (10μF 0805 25V X5R) | [`CL21A106KAYNNNE_C15850.pdf`](./CL21A106KAYNNNE_C15850.pdf) |
| `3-C1`, `3-C9` | **1206B475K500NT** | C29823 | 风华 (FH) | MLCC (4.7μF 1206 50V X7R) | [`1206B475K500NT_C29823.pdf`](./1206B475K500NT_C29823.pdf) |
| `2-C2` | **CL05A105KA5NQNC** | C52923 | 三星 (Samsung) | MLCC (1μF 0402 25V X5R) | [`CL05A105KA5NQNC_C52923.pdf`](./CL05A105KA5NQNC_C52923.pdf) |
| `0-C8` | **CL05B104KB54PNC** | C307331 | 三星 (Samsung) | MLCC (0.1μF 0402 50V X7R) | [`CL05B104KB54PNC_C307331.pdf`](./CL05B104KB54PNC_C307331.pdf) |
| `2-C4`, `3-C3`, `4-C6`, `5-C1` | **CL05B104KO5NNNC** | C1525 | 三星 (Samsung) | MLCC (0.1μF 0402 16V X7R) | [`CL05B104KO5NNNC_C1525.pdf`](./CL05B104KO5NNNC_C1525.pdf) |
| `1-C3` | **CL05B103KB5NNNC** | C15195 | 三星 (Samsung) | MLCC (0.01μF 0402 50V X7R, BOOT用) | [`CL05B103KB5NNNC_C15195.pdf`](./CL05B103KB5NNNC_C15195.pdf) |

### 3.4 抵抗 (Resistors)

| リファレンス | 部品型番 | LCSC 品番 | メーカー | 概要 / 定格 | ローカル PDF リンク |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `1-R2` | **0603WAF6801T5E** | C23212 | 厚声 (UNI-ROYAL) | チップ抵抗 (6.8kΩ 1% 0603, DCDC分圧) | [`0603WAF6801T5E_C23212.pdf`](./0603WAF6801T5E_C23212.pdf) |
| `1-R3` | **0603WAF2201T5E** | C4190 | 厚声 (UNI-ROYAL) | チップ抵抗 (2.2kΩ 1% 0603, DCDC分圧) | [`0603WAF2201T5E_C4190.pdf`](./0603WAF2201T5E_C4190.pdf) |
| `2-R1` | **0402WGF1003TCE** | C25741 | 厚声 (UNI-ROYAL) | チップ抵抗 (100kΩ 1% 0402, VDD_SWプルダウン) | [`0402WGF1003TCE_C25741.pdf`](./0402WGF1003TCE_C25741.pdf) |
| `R1`, `R2` | **0402WGF1002TCE** | C25744 | 厚声 (UNI-ROYAL) | チップ抵抗 (10kΩ 1% 0402, BOOT_REQ分圧/保護) | [`0402WGF1002TCE_C25744.pdf`](./0402WGF1002TCE_C25744.pdf) |
| `4-R5`, `4-R6`, `5-R1`, `R12`, `R13` | **0402WGF1001TCE** | C11702 | 厚声 (UNI-ROYAL) | チップ抵抗 (1kΩ 1% 0402, バイアス/LED/BMC) | [`0402WGF1001TCE_C11702.pdf`](./0402WGF1001TCE_C11702.pdf) |
| `4-R4` | **0402WGF1200TCE** | C25079 | 厚声 (UNI-ROYAL) | チップ抵抗 (120Ω 1% 0402, RS-485終端) | [`0402WGF1200TCE_C25079.pdf`](./0402WGF1200TCE_C25079.pdf) |
| `R14` | **0402WGF3300TCE** | C25104 | 厚声 (UNI-ROYAL) | チップ抵抗 (330Ω 1% 0402, LED-PB4制限) | [`0402WGF3300TCE_C25104.pdf`](./0402WGF3300TCE_C25104.pdf) |
| `4-R7`, `4-R8` | **0402WGF100JTCE** | C25077 | 厚声 (UNI-ROYAL) | チップ抵抗 (10Ω 5% 0402, RS-485直列ダンピング) | [`0402WGF100JTCE_C25077.pdf`](./0402WGF100JTCE_C25077.pdf) |

### 3.5 コネクタ・ヘッダ (Connectors & Headers)

| リファレンス | 部品型番 | LCSC 品番 | メーカー | 概要 / 定格 | ローカル PDF リンク |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0-P1` | **WJ25C-B-7.62-6P** | C14864 | 康奈克斯 (KANGNEX) | 7.62mm ピッチ 6P 端子台 (電源 & RS-485) | [`WJ25C-B-7.62-6P_C14864.pdf`](./WJ25C-B-7.62-6P_C14864.pdf) |
| `GPIO-IDC` | **Z-230011020209** | C221002 | 正凌 (Nextron) | 2.54mm 2×10P IDC コネクタ (ADX Pinout v2) | [`Z-230011020209_C221002.pdf`](./Z-230011020209_C221002.pdf) |
| `H1`, `H2`, `H3` | **2541WV-01P** | C25503131 | 瀚源 (Han Electricity) | 2.54mm 1P ピンヘッダ (FG, UPDI×2) | [`2541WV-01P_C25503131.pdf`](./2541WV-01P_C25503131.pdf) |
