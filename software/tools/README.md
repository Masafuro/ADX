<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Software Tools

ADX ハードウェア設計・製造・検査向けのユーティリティスクリプト群です。

## `gen_adx_datamatrix.py` — 8854 光学 Board ID 用 DataMatrix 生成ツール

ADX Official Optical Profile（`M4` スロット：`10×10` ECC200 DataMatrix / 6 桁 Board ID）用の、EasyEDA Pro / KiCad 取り込み対応 **SVG（結合ベクターパス）** および **透過 PNG** を生成します。

- 外部バーコードライブラリ不要（純 Python で ISO/IEC 16022 ECC200 `10×10` ＋ Reed-Solomon 符号化＋自動デコード自己検証を内蔵。PNG 出力時のみ Pillow を使用）
- 既定モード（`--mode silk`）では **黒＝シルク印刷 ON、透明＝レジスト地 OFF、ファイル内余白 `0 mm`** で出力するため、EasyEDA Pro へ **`4.0 × 4.0 mm`** 指定で貼り付けるだけで暗色レジスト基板では自動的に反転シルク（Light-on-Dark）、白レジスト基板では通常極性（Dark-on-Light）になります。

### 基本の使い方

```bash
# 標準仕様（4.0 x 4.0 mm / 0.40 mm/cell / 余白 0 / 反転シルク標準）
python3 software/tools/gen_adx_datamatrix.py 100010 -o hardware/CARD/Reference/data --preview

# 太めセル仕様（4.5 x 4.5 mm / 0.45 mm/cell）
python3 software/tools/gen_adx_datamatrix.py 000000 --cell-mm 0.45 -o out/

# シルクインク滲み補正（露出外周を片側 0.03 mm 絞る）
python3 software/tools/gen_adx_datamatrix.py 100010 --bleed-comp-mm 0.03 -o out/

# 比較実験用：外枠ベタ塗り＋レジスト抜き（方式2：2セル=0.8mm 白枠付き）
python3 software/tools/gen_adx_datamatrix.py 100010 --mode negative-box --quiet-cells 2 -o out/ --preview
```

### EasyEDA Pro への配置手順（`M4` スロット）

1. 生成された `adx_dm_<ID>_4.0mm.svg`（または `.png`）を `TopSilkscreen` 層へ `Place → Image` で取り込む。
2. 幅・高さを **`4.0 mm × 4.0 mm`**（`0.45 mm/cell` の場合は `4.5 mm × 4.5 mm`）、**極性反転（Invert）は OFF**（黒ピクセル＝シルク ON）に設定する。
3. 中心座標を **`(81.0, 14.0) mm`** に合わせる（周囲 `1.0 mm` が `6.0 × 6.0 mm` の予約領域余白となる）。
4. 両面実装で裏面にも配置する場合は、`BottomSilkscreen` 層へ同じく `(81.0, 14.0) mm` へ通常配置する（手動で左右反転しないこと）。
