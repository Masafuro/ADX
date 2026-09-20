<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# サブタスク 3: 基板製造性（DFM）レビュー仕様書 (Task 3: Fabrication DFM Review)

- **タスクID**: Task 3
- **対象**: 基板製造プロセス適合性、幾何公差、外形加工、ソルダーレジスト、シルク印刷
- **入力データ**:
  - `hardware/CORE-S/review/rev3/data/Gerber_PCB1_2026-09-19.zip`（全レイヤー: GTL, GBL, GTS, GBS, GTO, GBO, GTP, GBP, GKO, GDL, GDD, DRL）
  - JLCPCB 2層標準製造能力仕様（Minimum Trace/Space, Min Drill, Annular Ring 等）

---

## 1. タスク概要と目的

PCB 製造工場（JLCPCB 等）の標準製造公差・仕様と照らし合わせ、**製造保留（Engineering Question: EQ）の発生、歩留まり低下、あるいは基板加工不良を引き起こす要因をゼロにする** ための DFM（Design for Manufacturing）検証を行います。

---

## 2. 詳細レビュー項目とチェックリスト (Review Items)

### 2.1 パターン幅・クリアランス（Trace Width & Space）
- [ ] **最小パターン幅（Minimum Trace Width）**:
  - 全銅箔層（Top/Bottom）において、パターン幅が製造限界（JLCPCB標準: $\ge 5\,\mathrm{mil} \approx 0.127\,\mathrm{mm}$）を十分にクリアしているか（推奨: $6\,\mathrm{mil}$ 以上）。
- [ ] **最小クリアランス（Minimum Clearance / Spacing）**:
  - パターン間、パターン〜パッド間、パッド〜パッド間の間隙が製造限界（$\ge 5\,\mathrm{mil}$）をクリアしているか。
- [ ] **銅箔〜基板エッジクリアランス（Copper to Edge）**:
  - 基板外形線（Board Outline: GKO）から銅箔パターンまでの距離が、規定（$\ge 0.3\,\mathrm{mm}$ / $12\,\mathrm{mil}$）以上確保されているか（ルーター切削時のバリ・めくれ防止）。

### 2.2 ドリル・ビア仕様 (Holes, Vias & Annular Ring)
- [ ] **最小ドリル穴径（Minimum Drill Size）**:
  - ビアおよびスルーホールの最小ドリル径が、工場標準（$\ge 0.3\,\mathrm{mm}$、推奨 $0.3\,\mathrm{mm} / 0.6\,\mathrm{mm}$ ビア）を満たしているか。
- [ ] **最小アニュラリング（Minimum Annular Ring）**:
  - ドリル穴周囲の銅箔リング幅が、規定（$\ge 0.13\,\mathrm{mm} / 5\,\mathrm{mil}$）以上確保されているか（穴ズレ時の断線防止）。
- [ ] **PTH / NPTH の適切な分離**:
  - `Drill_PTH_Through.DRL`（メッキあり）と `Drill_NPTH_Through.DRL`（メッキなし取り付け穴等）が正しく分離定義されているか。
- [ ] **穴間クリアランス（Hole to Hole）**:
  - 隣接するドリル穴同士の間隔が $0.5\,\mathrm{mm}$ 以上確保されているか（基板クラック防止）。

### 2.3 基板外形・機械加工 (Board Outline & Slots)
- [ ] **基板外形線（GKO）の完全性**:
  - `Gerber_BoardOutlineLayer.GKO` が途切れのない単一の閉曲線（Closed Loop）で定義されているか。
  - 外形寸法が仕様通りのコンパクトサイズ（ADX 8748 フォームファクタ等）に収まっているか。
- [ ] **長穴・スロット加工の有無**:
  - 端子台やコネクタの異形ピン穴がある場合、GKO または GDL 層に正しく定義されているか。

### 2.4 ソルダーレジスト仕様 (Solder Mask)
- [ ] **微小ピッチパッド間のレジストダム（Solder Bridge Mask Dam）**:
  - `ATTINY1616`（0.4mm ピッチ QFN-20）および `TPS22965`（0.5mm ピッチ UDFN-8）のパッド間に、レジストダム（幅 $\ge 0.1\,\mathrm{mm}$）が形成できる開口設計になっているか。
- [ ] **サーマルパッドのレジスト開口**:
  - QFN や ESOP-8 の底面放熱パッドが適切に開口（Exposed）されているか。

### 2.5 シルク印刷品質 (Silkscreen Quality)
- [ ] **シルク文字サイズと線幅**:
  - 文字の高さ（$\ge 1.0\,\mathrm{mm}$）および線幅（$\ge 0.15\,\mathrm{mm} / 6\,\mathrm{mil}$）が工場推奨値を満たし、つぶれず読めるか。
- [ ] **パッド・テストポイントへのシルク被り（Overlap）**:
  - 部品のハンダ付けパッドやテストポイント上にシルク印刷が被っていないか（ハンダ不良・接触不良の防止）。
- [ ] **極性・ピン1マークの視認性**:
  - ダイオードのカソードマーク、IC の 1 番ピンマーク、コネクタのピン番号シルクが明確に配置されているか。

---

## 3. 検証手順と使用ツール

1. **ガーバー自動解析スクリプト**:
   - GKO、DRL、GTL、GBL、GTS、GBS、GTO、GBO をスクリプトで解析し、幾何限界値を抽出。
2. **DFM ルール違反検出**:
   - パターン幅 $< 5\mathrm{mil}$、クリアランス $< 5\mathrm{mil}$、ドリル $< 0.3\mathrm{mm}$ の座標を全数走査。
3. **シルク・レジスト開口オーバーラップ検出**:
   - GTO/GBO と GTS/GBS の積集合（重複領域）を算出し、パッド上のシルクをリストアップ。

---

## 4. 完了基準 (Definition of Done)

- [ ] JLCPCB 等の 2 層標準製造基準に対する全項目の照合が完了していること。
- [ ] 製造保留（EQ）リスクとなる要素がゼロであることが確認されていること。
- [ ] 検証結果が `hardware/CORE-S/review/rev3/result/task3_result.md` に取りまとめられていること。
