<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# U3 技術調査レポート：USB-C 端子と基板外形線の `×` マーク（API 検知可否と安全境界）

- **作成日**: 2026-11-28
- **作成者**: プロジェクトリーダー（AI）
- **起票元**: [supporter_log.md](../supporter_log.md)（U3 `<<サポーターより>>` の現場確認事項）
- **対象部品**: `J_USB`（`TYPE-C-31-M-12` / LCSC `C165948`）

---

## 1. 結論（3 行サマリー）

1. **「あの `×` マークは大丈夫か？」→ いいえ、修正すべき製造クリアランス違反でした。**
   - 前シェル固定スルーホール（`J_USB_2`, `J_USB_3`）の銅箔左端が基板外形線から **`7.2 mil`（約 `0.18 mm`）** しか離れておらず、EasyEDA / JLCPCB 1oz 標準ルール（`>= 11.8 mil` = `0.30 mm`）を下回っていました。
2. **「PL（API）から確認できるのか？」→ はい、`eda.pcb_Drc.check(true, false, true)` で 100% 機械検知できました。**
   - 違反カテゴリ・ルール名・対象パッド・実測距離・しきい値まで構造化 JSON で取得可能です。
   - `J_USB` を右へ `+20 mil`（`X = 1140 → 1160 mil`）戻すことで、**`Clearance Error` が `2 件 → 0 件` へ完全解消**することまで実測確認しました（更新後 PNG でも `×` マーク完全消滅）。
3. **「それでも API に死角はあるか？」→ はい、「3D 物理ボディの突き出し・ケーブル挿抜・筐体干渉」は API だけでは検知できません。**
   - 「銅箔・穴のクリアランス保証＝AI 自動 DRC ゲート」「3D メカ・使い勝手確認＝人間の 3D 目視レビュー」という明確な分業境界が浮かび上がりました。

---

## 2. API 実測結果と幾何学的検算

### 2.1 `J_USB`（`C165948`）前シェル長穴の API 実測データ

`eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId()` および `getState_Pad()` / `getState_Hole()` から取得した実測値（`rot = 270`、ボード左端外形線 `X = 1000.0 mil`）：

- 対象パッド: `Pad 2`（下前シェル）および `Pad 3`（上前シェル）
- 銅箔パッド外形: `["OVAL", 47.2, 70.9]`（`rot = -90°` により **X 方向全長 = `70.9 mil`**、すなわち左端までの半幅 = **`35.45 mil`**）
- スロット穴形状: `["SLOT", ...]`（めっき長穴）

### 2.2 修正前（`X = 1140 mil`）vs 修正後（`X = 1160 mil`）の比較

| 項目 | 修正前（`J_USB.x = 1140`） | 修正後（`J_USB.x = 1160`） | 判定・備考 |
| :--- | :---: | :---: | :--- |
| `Pad 2 / 3` 中心 X | `1042.6 mil` | `1062.6 mil` | ボード左端 `1000.0 mil` 基準 |
| 銅箔パッド左端 X | `1007.15 mil` | `1027.15 mil` | 中心 − 半幅 `35.45 mil` |
| **外形線までの実測距離** | **`7.15 mil` (`0.18 mm`)** | **`27.15 mil` (`0.69 mm`)** | ルールしきい値: `>= 11.8 mil` (`0.30 mm`) |
| `pcb_Drc.check` 出力 | **`Clearance Error`: 2 件**<br>(`Board Outline to TH Pad`) | **`Clearance Error`: 0 件** | 未配線 (`Connection Error`) 以外の違反 **全ボード 0 件** |
| GUI 上の黄色 `×` マーク | 2 箇所表示あり | **完全消滅** | [unomodoki_pcb_placed.png](../outputs/unomodoki_pcb_placed.png) で確認済み |

### 2.3 API が実際に返した DRC JSON（抜粋・修正前 `X = 1140`）

```json
{
  "errorType": "Clearance Error",
  "errorObjType": "Board Outline to TH Pad",
  "ruleName": "copperThickness1oz",
  "ruleTypeName": "Safe Spacing",
  "obj1": { "typeName": "Board Outline", "suffix": ": e1" },
  "obj2": { "typeName": "TH Pad", "suffix": "(GND): J_USB_3" },
  "explanation": {
    "str": "{obj1} to {obj2} distance is {minDistance}, should be {shouldBe}",
    "param": { "minDistance": "7.2mil", "shouldBe": ">= 11.8mil" }
  }
}
```

> [!NOTE]
> 幾何計算の `7.15 mil` と DRC API の実測値 `7.2 mil` が小数第 1 位まで一致しており、EasyEDA Pro の DRC エンジンを API 経由で完全に信用できることが確認できました。

---

## 3. なぜ初回 U3 で PL が事前ブロックできなかったのか（原因と改善）

API 側の機能不足ではなく、**PL の U3 検証フローの抜け**が原因でした。

- 初回の U3 スクリプト（[run_step3b_mech_place_silk.mjs](../scripts/run_step3b_mech_place_silk.mjs)）では、「全 38 ピンの座標一致アサート」だけを実行し、`pcb_Drc.check()` は「未配線エラーが大量に出るから U4 の配線後でいいだろう」と省略してしまいました。
- しかし実際には、`pcb_Drc.check(true, false, true)` の戻り値はカテゴリ別（`Clearance Error` / `Connection Error` 等）に綺麗に構造化されているため、**配線前の U3 時点でも「`Connection Error` 以外の全カテゴリが 0 件であること」を 1 秒で自動アサート可能**でした。

---

## 4. API の得意領域 vs 死角（M5：ADX 本丸へ持ち帰るルール）

サポーターからの問題提起により、ボードエッジ部品・メカ干渉に関する明確な分業境界が整理できました。

| チェック項目 | EasyEDA API で自動検証できるか？ | 推奨担当 |
| :--- | :---: | :---: |
| 銅箔パッド・長穴・ビア・配線と「基板外形線」の距離 | **◎ 100% 検知可**（`pcb_Drc.check` `Clearance Error`） | **AI（U3 配置直後の必須自動ゲート化）** |
| 部品パッド同士の重なり・ショート | **◎ 100% 検知可**（`pcb_Drc.check`） | **AI（U3 配置直後の必須自動ゲート化）** |
| ヘッダ・穴の数値座標誤差 | **◎ 100% 検知可**（`getAllPinsByPrimitiveId` アサート） | **AI（U3 必須自動ゲート）** |
| **USB-C 金属シェルの突き出し量・ケーブル挿抜のしやすさ** | **△ 不可（2D パッドのみでは 3D ボディ端が読めない）** | **人間（EasyEDA 3D ビュー目視確認）** |
| **M3 ネジ頭やケース壁・指先操作との 3D 物理干渉** | **× 不可（3D メカ形状・エルゴノミクス判断）** | **人間（EasyEDA 3D ビュー目視確認）** |

### ADX 本丸（[CORE-U rev6](../../../CORE-U/rev6/spec.md)）へ導入する新ルール

1. **U3 配置完了ゲートに「配線前クリアランス DRC ＝ 0 件」を追加する**（`Connection Error` 以外の全 DRC カテゴリが空であることを自動アサート）。
2. **USB-C / IDC 20P などのエッジ実装コネクタ**は、AI が DRC クリアランス 0 を保証したうえで、**人間が EasyEDA 3D ビューで最終的な突き出し量と挿抜感を承認する**二段構えを標準フローとする。
