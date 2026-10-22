<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Hackaday.io 投稿 トーン＆マナー ガイドライン
**〜 グローバル・ハードウェアハッカーに響く、リアルで誠実な開発ログの執筆指針 〜**

- **策定日**: 2026-10-22
- **ステータス**: APPROVED / 運用指針
- **対象**: `docs/blog/posts/` 配下の全原稿、および Hackaday.io 向け Project Log
- **上位方針**: [`memo/REPORT/WORKING_ARDUINO_TONE_AND_MANNER.md`](../../memo/REPORT/WORKING_ARDUINO_TONE_AND_MANNER.md)

---

## 1. はじめに：Hackaday.io のカルチャーを理解する

Hackaday.io は、世界中のハードウェアハッカー、組込エンジニア、メイカーが集うコミュニティです。  
彼らが最も愛するのは、**「綺麗なカタログスペック」ではなく、「実験ベンチでのリアルな試行錯誤と泥臭いハック」** です。

```text
［ 一般の企業ブログ / PR ］                  【 Hackaday.io の DevLog 】
  ・完成した製品の自慢・宣伝                   ・開発途中の生々しい実験と失敗談
  ・商業的バズワードで飾る                     ・オシロスコープの波形と生々しい基板写真
  ・「完璧」「世界初」と謳う                   ・「ここで行き詰まり、こうやって切り抜けた」
  ・完成品を買わせるのがゴール                 ・知見をシェアし、コメント欄で議論するのがゴール
```

ADXのコア精神である **「はたらくArduino（The Working Arduino）」**（机の上のプロトタイプを、現場のリアルな箱へ連れ出すための等身大の相棒）は、まさにこのハッカー文化と完全に共鳴します。

---

## 2. コア・トーンの4大原則 (Tone of Voice)

```text
┌────────────────────────────────────────────────────────────────────────┐
│                    【 Hackaday.io 執筆 4大原則 】                      │
│                                                                        │
│  1. 【 Lab Notebook 】  プレスリリースではなく「研究ノート」の語り口   │
│  2. 【 Honest Engineering 】 誇張を排し、トレードオフと数値を誠実に開示 │
│  3. 【 Story-Driven 】   「壁 (Challenge) ➔ 格闘 (Hack) ➔ 学び」の構成  │
│  4. 【 Community-First 】 完成を誇らず、仲間の知恵を歓迎する問いかけ   │
└────────────────────────────────────────────────────────────────────────┘
```

### 原則 1: Lab Notebook（研究ノートの語り口）
- **一人称**: `"We"`（開発チームとして）または `"I"`（作業者視点）。
- **距離感**: ガレージや研究室の作業机で、隣の席のエンジニア仲間に「今週こんな面白い現象に出くわしたんだよ」とコーヒー片手に語りかけるようなフレンドリーさ。
- **無味乾燥な進捗箇条書きにしない**: 「〇〇を実装した」だけで終わらせず、「なぜそれが必要で、どんな手応えだったか」のコンテキストを添えます。

### 原則 2: Honest Engineering（誠実な工学主義）
- **誇大広告・絶対表現の徹底排除**:
  - ✕ `"Revolutionary"`, `"Game-changer"`, `"100% bug-free"`, `"The Ultimate Arduino Killer"`
  - ◯ `"Tougher alternative"`, `"Practical workaround"`, `"Trade-offs we accepted"`
- **トレードオフを堂々と語る**:
  - 「外付け回路を足せば簡単だが、基板面積とBOMコストを抑えるためにファームウェアのレジスタ操作で吸収した」など、**エンジニアとしての意思決定の理由** を書くことで、プロの読者から絶大な信頼を得られます。

### 原則 3: Story-Driven（ストーリーテリング）
1本の記事には、必ず **「小さな起承転結（ドラマ）」** を持たせます：
1. **The Hook**: 最初の1行で「何に挑んだか」を一言で提示。
2. **The Challenge**: ぶつかった壁（ピン不足、ノイズ、タイミングずれ、熱など）。
3. **The Experiment / Hack**: 測定データ、波形、試行錯誤の過程。
4. **Key Takeaway**: 得られた知見（他のメイカーにも役立つアドバイス）。

### 原則 4: Community-First（コミュニティとの対話）
- 記事の最後には、必ず **読者への問いかけ（Call to Conversation）** を配置します。
- 「みんなならこのピン配置どうする？」「UPDIのノイズ対策で似たような経験ある？」など、オープンに意見を求める姿勢がフォロワーやSkull（いいね）を集めます。

---

## 3. 記事構成の黄金パターン (The Anatomy of a Log)

1記事あたり **300〜600ワード程度** の、サクッと読めて技術的密度の高いボリュームが最適です。

```markdown
<!-- 1. Hero Image & Lead -->
大判写真（16:9） + 今週のハイライトを凝縮した太字リード文（1〜2文）

<!-- 2. The Challenge (問題の提示) -->
## The Challenge: [キャッチーかつ技術的な小見出し]
「何を作ろうとして、どんな壁にぶつかったか」
抽象論ではなく、具体的なピン名・IC型番・電圧・プロトコル名を書く。

<!-- 3. How We Solved It (ハックの過程と測定) -->
## How We Solved It: [解決アプローチ]
オシロ波形や基板写真、10〜20行の要点コードスニペットを配置。
「なぜこの設計を選んだのか」を解説。

> **Key Takeaway:** [読者が明日から使える設計のコツを一言で]

<!-- 4. Weekly Progress Summary (進捗まとめ) -->
## Weekly Progress Summary
- **Hardware:** 基板アートワーク修正、BOM精査など
- **Firmware:** レジスタ設定、ドライバ実装など
- **Tools:** ブラウザ診断ツール (WebSerial Explorer) の更新など

<!-- 5. Next Steps & Community Thoughts (次回予告と問いかけ) -->
## Next Steps & Community Thoughts
来週予定している実験や製作。
読者への技術的な質問・フィードバックの募集。

<!-- (スクリプトにより CTA フッターが自動挿入されます) -->
```

---

## 4. 用語・表現 対比マトリクス (DO & DON'T)

Hackaday.io 向けに英語原稿（または翻訳ベースの原稿）を執筆する際の対比表です。

| カテゴリ | ✕ 避けるべき表現 (DON'T) | ◯ 推奨される表現 (DO) | 狙い・理由 |
| :--- | :--- | :--- | :--- |
| **製品アピール** | The ultimate industrial Arduino killer | A field-ready, enclosure-native companion for Arduino sketches | 敵対的・誇大なアピールを排し、「現場で使える相棒」としての価値を伝える。 |
| **品質・信頼性** | Flawless / 100% unbreakable | Robust against industrial noise / Tested for harsh edge conditions | ハードウェアに「絶対」はない。工学的に正確な表現にとどめる。 |
| **進捗報告** | We fixed several bugs and it works now. | Tuning slew-rate registers dropped ringing by 40%, stabilizing serial sync. | 具体的な数値・測定結果・メカニズムを語る。 |
| **回路・ファーム** | We invented a revolutionary way to debug MCU. | A neat firmware trick: repurposing USART break signals for bootloader entry. | 「革命」ではなく「気の利いたハック (neat trick)」として親しみやすく共有。 |
| **課題の開示** | Everything went smoothly as planned. | We hit a painful pin conflict between UPDI and Reset line. Here is how we tackled it. | 順風満帆な嘘より、苦闘と解決プロセスこそがハッカーの大好物。 |
| **読者への結び** | Check out our project and buy it. | What’s your preferred way to route UPDI in high-EMI setups? Drop your thoughts below! | 商業的売り込みを排し、技術談義の場を開放する。 |

---

## 5. ビジュアル（写真・コード・図）の鉄則

Hackaday の読者は **ビジュアルでスクロールの手を止めます**。

1. **生写真 ＞ CAD レンダリング**:
   - 3D CAD の綺麗なレンダリング画像だけでは響きません。
   - **「オシロスコープのプローブが何本も刺さったブレッドボード」「はんだ付けされたプロト基板」「測定器の生画面」「3Dプリントケースに実際にねじ止めした写真」** が最高のコンテンツです。
2. **コードは「急所」の 10〜20行 に絞る**:
   - 何百行ものソースコードをベタ貼りしない。
   - レジスタ設定や、タイミング制御のコアロジックだけを抜き出し、前後にコメントを添えます。
   - 全体コードは GitHub へのリンクで誘導します。
3. **キャプションに技術情報を込める**:
   - ✕ `Fig 1: Circuit board`
   - ◯ `Fig 1: ATTiny1616 break timing capture showing the 120µs pull-down pulse`

---

## 6. 公開前セルフチェックリスト (Pre-flight Checklist)

記事をビルドして公開する前に、以下の 6 項目をチェックしてください：

- [ ] **1. 誇大表現はないか？** (`revolutionary`, `perfect`, `100%`, `unbreakable` 等が入っていないか)
- [ ] **2. 「壁（Challenge）」と「知見（Key Takeaway）」が含まれているか？** (単なる進捗リストになっていないか)
- [ ] **3. 生の技術エビデンス（波形写真、基板写真、測定値、重要コード）があるか？**
- [ ] **4. 語り口は親しみやすいエンジニア目線（Lab Notebook）になっているか？**
- [ ] **5. コミュニティに対する問いかけが末尾にあるか？**
- [ ] **6. 画像の最適化と GitHub (main) へのプッシュが完了しているか？** (jsDelivr CDN 経由で画像が配信されます)
