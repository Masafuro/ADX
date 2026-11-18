# Stage 1 Reference: Episode 02 — 12V HV UPDI on 20-Pin IDC Bus

本エピソードの Stage 1（v1 対話素材）は、既存の [v1/script_idc_updi.md](../archive/v1/script_idc_updi.md) をそのまま使用しています。

## Stage 1 から抽出された4要素（Stage 2 変換用チェックリスト）

1. **The Taboo（禁忌）**:
   * シールド無しの20ピンフラットケーブル（Pin 14）に、12V高電圧パルスが走り得る UPDI を統合した。
2. **The Objections（ECHOの反論 → 聴き手の疑問へ変換）**:
   * 3.3Vロジックの真横に12V（ミニチュアの雷）を流せば、誤動作や誤接続でバス全体が魔法の煙（Magic Smoke）を吹いて死ぬ。
   * 本人自身も3ヶ月間は「絶対に入れない」と言っていた方針転換。
3. **The Metaphor Bites（残すべき比喩）**:
   * *"a miniature lightning bolt next to fragile 3.3V lines"*（3.3Vの真横を走るミニチュアの雷）
   * *"a bed-of-nails fixture is a medieval torture device"*（針山ポゴピン治具＝中世の拷問器具）
   * *"armed every single chip with a bomb shelter and a riot shield"*（全チップに核シェルターと暴徒鎮圧用シールドを配備）
4. **The Reality & Math Payoff（逆転の事実と数値）**:
   * ATtiny1616（3mm VQFN）× OSP基板の酸化パッドで、ポゴピン検査は接触抵抗スパイクによる **14%の書き込み偽エラー** が発生。
   * 全ノードに専用ESDクランプアレイ＋直列制限抵抗を実装（3V3トレラント化）。
   * 12V直撃時でも電流は **10mA以下** に制限、クランプ応答 **3.8ns**、熱許容量内。
   * 結びの哲学: *"Brute force that works is called engineering."*
