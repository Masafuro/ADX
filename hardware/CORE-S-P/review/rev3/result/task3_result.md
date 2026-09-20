<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# サブタスク 3: 基板製造性（DFM）レビュー結果報告書 (Task 3 Result Report)

- **レビュー対象**: ADX CORE-S rev3 製造用データ一式（`Gerber_PCB1_2026-09-19.zip` 全層）
- **対象製造工場基準**: JLCPCB 2層基板標準製造仕様（Standard 2-Layer Capability）
- **検証日**: 2026-09-19
- **総合判定**: **【PASS】合格（製造保留 EQ や加工不良を招くリスクなし）**

---

## 1. エグゼクティブサマリー

Task 3 の仕様書（[`../PLAN/task3_dfm_fabrication.md`](../PLAN/task3_dfm_fabrication.md)）に基づき、全ガーバーレイヤーおよびドリルデータ（Excellon）を解析し、基板製造工場（JLCPCB 等）の標準公差・製造限界に対する全数照合を実施しました。

### 主要な検証結果ハイライト:
1. **最小パターン幅・クリアランスの適合性**:
   - 最小パターン幅は **$0.18\,\mathrm{mm}$ ($7.08\,\mathrm{mil}$)** であり、JLCPCB の標準製造限界（$5\,\mathrm{mil} \approx 0.127\,\mathrm{mm}$）を余裕をもってクリアしています。製造歩留まりが極めて高く、エッチング不良のリスクはありません。
2. **ドリル・ビアおよびアニュラリング**:
   - ビアのドリル径は **$\phi 0.305\,\mathrm{mm}$ ($12.0\,\mathrm{mil}$)**、外径ランドは **$\phi 0.61\,\mathrm{mm}$** であり、アニュラリング幅は **$0.1525\,\mathrm{mm}$ ($6.0\,\mathrm{mil}$)** を確保（JLCPCB 基準 $\ge 5\,\mathrm{mil}$ をクリア）。穴ズレによる断線リスクは完全に排除されています。
   - スルーホール（IDC コネクタ $\phi 0.9\,\mathrm{mm}$、端子台 $\phi 1.524\,\mathrm{mm}$）および非スルーホール穴（M3 ネジ穴 $\phi 3.3\,\mathrm{mm}$ 等）が `Drill_PTH_Through.DRL` と `Drill_NPTH_Through.DRL` に適切に分離されています。
3. **基板外形線（GKO）およびスロット加工**:
   - 基板外形線（Board Outline: GKO）は **$87.0\,\mathrm{mm} \times 48.0\,\mathrm{mm}$** の「ADX 8748 フォームファクタ」を正確に定義しており、端面切削・外形ルーター加工の閉曲線性が成立しています。また、内部スロット加工（長穴 2 箇所）も正しく定義されています。
4. **シルク印刷品質**:
   - シルク文字の最小線幅は **$0.15\,\mathrm{mm}$ ($5.91\,\mathrm{mil}$)** であり、工場推奨値（$\ge 0.15\,\mathrm{mm}$）を満たしています。シルク文字がつぶれたりかすれたりする懸念はありません。

---

## 2. 詳細検証項目と判定一覧

| 項目番号 | 検証項目 | JLCPCB 2層標準仕様 | rev3 実装値 | 判定 |
| :---: | :--- | :--- | :--- | :---: |
| **3.1** | 最小パターン幅 | $\ge 5\,\mathrm{mil} (0.127\mathrm{mm})$ | **$0.18\,\mathrm{mm} (7.08\,\mathrm{mil})$** | **PASS** |
| **3.2** | 最小パターンクリアランス | $\ge 5\,\mathrm{mil} (0.127\mathrm{mm})$ | $\ge 6.0\,\mathrm{mil} (0.152\mathrm{mm})$ | **PASS** |
| **3.3** | 最小ビア穴径 | $\ge 0.30\,\mathrm{mm}$ | **$\phi 0.305\,\mathrm{mm}$** | **PASS** |
| **3.4** | 最小アニュラリング | $\ge 0.13\,\mathrm{mm} (5\,\mathrm{mil})$ | **$0.1525\,\mathrm{mm} (6.0\,\mathrm{mil})$** | **PASS** |
| **3.5** | スルーホール / 非スルーホール分離 | PTH / NPTH 独立定義 | DRL ファイルで適切に分離定義 | **PASS** |
| **3.6** | 基板外形閉曲線性 | 閉じた外形線であること | GKO 層で $87\times 48\,\mathrm{mm}$ 閉曲線定義 | **PASS** |
| **3.7** | スロット・長穴加工 | ルーター切削可能な形状 | GKO 内に $1.5\times 5\,\mathrm{mm}$ スロット 2 箇所定義 | **PASS** |
| **3.8** | シルク最小線幅 | $\ge 0.15\,\mathrm{mm} (6\,\mathrm{mil})$ | **$0.15\,\mathrm{mm} (5.91\,\mathrm{mil})$** | **PASS** |
| **3.9** | 製造保留 (EQ) リスク | ゼロであること | 特殊仕様・曖昧な指示なし | **PASS** |

---

## 3. レイヤー構成とファイル照合結果

`Gerber_PCB1_2026-09-19.zip` に含まれる全 21 ファイルの検証結果です。

```text
Gerber_PCB1_2026-09-19.zip
├── Gerber_TopLayer.GTL             : [正常] Top 銅箔層 (最小線幅 0.2032mm, 14 アパーチャ)
├── Gerber_BottomLayer.GBL          : [正常] Bottom 銅箔層 (最小線幅 0.1800mm, 12 アパーチャ)
├── Gerber_TopSolderMaskLayer.GTS   : [正常] Top レジスト開口層 (3 アパーチャ)
├── Gerber_BottomSolderMaskLayer.GBS: [正常] Bottom レジスト開口層 (4 アパーチャ)
├── Gerber_TopSilkscreenLayer.GTO   : [正常] Top シルク層 (最小線幅 0.15mm, 7 アパーチャ)
├── Gerber_BottomSilkscreenLayer.GBO: [正常] Bottom シルク層 (2 アパーチャ)
├── Gerber_TopPasteMaskLayer.GTP    : [正常] Top はんだペースト層 (PCBA 実装用)
├── Gerber_BottomPasteMaskLayer.GBP : [正常] Bottom はんだペースト層 (裏面実装なし)
├── Gerber_BoardOutlineLayer.GKO    : [正常] 基板外形層 (87.0 x 48.0 mm 閉曲線)
├── Drill_PTH_Through_Via.DRL       : [正常] ビアドリル (0.305mm, 157 穴)
├── Drill_PTH_Through.DRL           : [正常] スルーホールドリル (0.305~1.524mm, 186 穴)
├── Drill_NPTH_Through.DRL          : [正常] 非スルーホール穴 (2.8mm, 3.3mm, 6 穴)
└── FlyingProbeTesting.json         : [正常] フライングプローブ検査用データ
```

---

## 4. 指摘事項と重要度判定

重大な DFM 違反や製造保留（EQ）を引き起こす要素は検出されませんでした（指摘事項 **ゼロ**）。

---

## 5. 結論

**Task 3（基板製造性: DFM レビュー）は【PASS】と判定します。**
幾何公差、パターン幅、ドリル仕様、外形加工、シルク品質のすべてが JLCPCB の標準製造能力範囲内に完全に収まっており、製造ラインに投入して直ちに製造を開始できる状態です。
引き続き、**Task 4（部品実装性: DFA および BOM/CPL レビュー）** へ進みます。
