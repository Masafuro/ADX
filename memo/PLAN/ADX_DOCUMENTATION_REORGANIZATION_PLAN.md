<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ドキュメント再編成計画書
— CORE-U を主役に据えた「Arduino派生・筐体親和型ボード」への転換 —

- **策定日**: 2026-10-12
- **ステータス**: DRAFT / 改訂版
- **対象ディレクトリ**: `memo/PLAN/ADX_DOCUMENTATION_REORGANIZATION_PLAN.md`
- **主要対象ドキュメント**: `README.md`（ルート英語）、`docs/ja/README.md`、`docs/ja/adx_formfactor_guidelines.md`、`hardware/CORE-U/concept.md`

---

## 1. 背景と目的（Context & Strategic Objective）

### 1.1 背景：パラダイムの地殻変動
Crowd Supplyの事例研究およびReddit（特に r/Arduino）コミュニティの生態調査から、従来の**「産業用・現場向け（CORE-A + RS-485 + 専用プローブRPR4）」を主軸としたアプローチは、一般メイカーにとって参入障壁が極めて高く、コミュニティ形成が困難である**という重大な課題が判明した。

また、「後工程はお客様」「ムリ・ムラ・ムダのないこと」という品質管理・工学の原点に立ち返った結果：
1. **「産業用の看板」はきっちり下ろし、「Arduinoの新しい派生ボード」としての親しみやすい顔で世に出るべきである**
2. **CORE-U はすでに CH342K + UPDI による Arduino IDE 開発環境が安定動作している**
3. **世界中のメイカーが直面する本質的な課題は「ケースに入れた時の配線スパゲッティ」と「電源の衝突・容量不足」である**

という結論に至った。

### 1.2 目的
ADXの看板を「難解な産業用」から**「ケースに美しく収まる、Arduino派生開発ボードのエコシステム（An Enclosure-Friendly Arduino-Compatible Board）」**へと刷新し、r/Arduinoコミュニティおよび来春の Crowd Supply に向けて、ドキュメント群を根本から再編成する。

---

## 2. 編集における「4大原則」と配慮事項

### 原則 1: 産業用の看板は「きっちり下ろす」（Arduino派生ボードの顔を徹底）
- トップページやメインの紹介から「産業用」「工業規格」「PLC」「Modbus」などの重い言葉を完全に撤去する。
- **「Arduinoの新しい派生ボード（Derivative Board）」としての顔を最前面に出す**。
- CORE-A（耐熱、絶縁RS-485など）は主題には一切持ってこず、**ドキュメント末尾のロードマップや将来展望に「将来的には、より過酷な環境や現場で使える上位ラインナップも登場するかも？」とこっそり添える程度にとどめる**。

### 原則 2: 「筐体フレンドリー（Enclosure-Friendly）」の真の定義（あらゆる筐体への組付け性）
- 「特定の既製品ケースにピッタリ入る」という狭い最適化ではなく、**自作ボックス（3Dプリント）、既製品ケース、電工・防水ボックスなど、あらゆる筐体への組付け・組み込み設計を容易にするための3大要素**として定義する：
  1. **固定・保持の自由度（Mechanical Fixation & Clearance）**:
     - 4隅の明確なM3ネジ穴（77.0 mm × 38.0 mm ピッチ）と、ネジ頭・ワッシャーがパターンを傷つけない十分な**ワッシャークリアランス（Φ5.0 mm逃げ幅）**の確保。
     - レールへの**スロットイン（Slot-in）**や3Dプリント爪での**スナップフィット（Snap-fit）**に対応する、基板外周の**「外形部掴み幅（エッジマージン）」**の確保。
  2. **I/F（インターフェース）辺の明示的な規格化（Unidirectional Interface Edge）**:
     - コネクタが四方八方に飛び出す「四方突出し問題」を排除し、外部接続（Type-C×2ポート等）を**単一の「I/F辺」に集約**。
     - これにより、ケースの穴あけ加工や3Dプリント開口部の設計が「1面だけ」で完結する。
  3. **IDCケーブルを使った拡張の規格化による柔軟な筐体内配置（Flexible Topology via IDC Cable）**:
     - 20P拡張バスは、基板同士の直スタックだけでなく、**市販のIDCフラットリボンケーブル接続を標準規格化**。
     - 薄型ケースなら「横並び」、深型ケースなら「上下スタック」、段違いボスなら「ケーブルで逃がす」など、筐体内部の空間形状に合わせて自由自在にレイアウト可能。

### 原則 3: 「Arduino UNO 代替」とは言わない（誠実なスペックの提示）
- 「Arduino UNOでできることはCORE-Uでだいたいできる」というのは感覚的な目安に過ぎず、原理的なMCUの差異がある：
  - Flash容量の差（ATmega328P 32KB vs ATtiny1616 16KB）
  - ピン数・ペリフェラル構成の違い
  - `megaTinyCore` の適合性や、Uno専用レジスタ直叩きライブラリの非互換リスク
- したがって、**「UNOを代替できる」と明示的に謳うことは避け、技術的に誠実な表現を貫く**：
  - 「Arduino IDEで手軽にプログラムできる、ATtiny1616ベースの軽量・コンパクトな派生ボード」
  - 「小〜中規模の制御・センサー・モーター駆動プロジェクトに特化した、筐体親和型ボード」

### 原則 4: CORE-U の位置づけ（確実なコア体験への集約と個別ドキュメントへの誘導）
- CORE-U は現在仕様策定段階にあり、電源回路（理想ダイオード構成やType-Cポート数など）はコストや部品入手性の観点から検証中である。
- したがって、トップレベル（README等）で流動的な回路仕様を断定的に書き込まず、以下の**確実なコアバリューに落とし込む**：
  1. **Arduino IDE ＋ megaTinyCore ネイティブ**: ブートローダー不要、手持ちの環境から即座に開発可能。
  2. **USB Type-C による手軽な接続**: ケーブル1本で開発・給電。
  3. **ADX規格（8748外形 / 20Pバス）への完全準拠**: ケースへの収まりやすさとCARD拡張性。
- 電源保護回路やコネクタ構成等の詳細仕様については、**「詳細は各ハードウェアドキュメント（`hardware/CORE-U/`）を参照」**という形でリンク・誘導し、トップページの記述が仕様変更で陳腐化しないようにする。

---

## 3. 文章表現リファレンス（Style & Voice Guidelines）

一見矛盾する**「エンジニアとしての誠実さ（事実ベース・誇張の排除）」**と**「r/Arduino初学者でもスッと頭に入る平易さ」**を統合するための執筆スタイルガイド：

### 3.1 コア・トーン：「誠実な平易さ（Honest Simplicity）」
- **誇張しないからこそ、言葉が素直に伝わる**: マーケティング的な煽り文句（ハイプ）や威圧的な専門用語を一切使わず、等身大の言葉で「何ができて、何ができないのか」を淡々と、かつ温かみをもって語る。
- **読者像**: 「ブレッドボードでLEDはチカチカできたけれど、ケースに入れる方法や電源の繋ぎ方で悩んでいる」世界中のメイカーや学生。

### 3.2 表現の4大ルール

1. **ビジュアルファースト ＆ ポジティブな事実提示（ペイン煽りの排除）**:
   - マーケティング的な「こんな悩みありませんか？」「〜で困っていませんか？」といううさんくさいペイン煽りやネガティブ表現を一切使わない。
   - 最初の「うわ、これ欲しい！」という直感的な魅力は、冒頭の**アイキャッチ画像・写真（現在はNo Photo枠を配置）**に担当させる。
   - テキストは最初から「ADX CORE-Uとは何か」「どんな美しい工夫が詰まっているか」を、ポジティブかつ誇張なく語る。
2. **大げさな表現・バズワードの完全排除（No Hype, Just Facts）**:
   - "Revolutionary", "Ultimate", "Indestructible", "Next-Gen Standard", "Arduino Killer" などの誇大語は使用禁止。
   - 具体的で控えめな事実をそのまま書く。
   - 例: ×「革命的な電源アーキテクチャ」 ➔ ◯「PCを繋いだまま、隣からサーボに3A給電できるDual USB」
3. **制約とスコープを誠実に開示する（Radical Candor）**:
   - できないこと、苦手な領域を隠さず正直に書くことで、技術者としての深い信頼を獲得する。
   - 例: *「ATtiny1616（Flash 16KB）を採用しているため、大きなディスプレイ描画や重量級ライブラリには向きません。しかし、センサーを読んでサーボやリレーを動かすといった小〜中規模のプロジェクトには、驚くほど手軽で頑丈です。」*
4. **技術的な工夫は「なぜそうしたか（Why）」を平易に解説する**:
   - 回路の難解な部品型番を押し付けるのではなく、それがもたらすユーザーの安心感を平易な言葉で説明する。
   - 例: *「PCを壊さないよう、小さな理想ダイオードを挟んでいます。ACアダプタから高電圧が来ても、PC側へ電気が逆流することはありません。」*

### 3.3 表現対比表（DOs vs DON'Ts）

| 避けるべき表現（DON'T） | 推奨する表現（DO） | 理由・トーンの意図 |
| :--- | :--- | :--- |
| **"The Ultimate Industrial Standard"**<br>（究極の産業用標準規格） | **"An Enclosure-Friendly Arduino Board"**<br>（ケースに優しいArduino派生ボード） | 威圧感を排し、親しみやすい入り口にする。 |
| **"Completely replaces Arduino UNO"**<br>（Arduino UNOを完全代替） | **"Great for small-to-medium control tasks"**<br>（小〜中規模の制御に最適） | MCUスペックの差やライブラリ互換性に誠実であるため。 |
| **"Galvanically isolated dual-feed architecture"**<br>（ガルバニック絶縁デュアル給電アーキテクチャ） | **"Dual USB: Safe debugging while powering big servos"**<br>（大電流サーボを動かしながら安全にデバッグできる2つのUSB） | 初学者でも「何が嬉しいのか」が1秒で理解できる。 |
| **"Perfect fit for Takachi cases"**<br>（タカチケースに完璧にフィット） | **"Designed for easy mounting in 3D prints and boxes"**<br>（自作3Dプリントや既製ボックスに組み込みやすい設計） | 既製品にポン付けできるという誤解を防ぐ。 |

---

## 4. 製品ポジショニングの再編（Hierarchy）

「ハードウェア規格（Form Factor & Bus）」という共通の土台の上に、旗艦ボード「CORE-U」と拡張「CARD群」が乗るという明確な階層関係を定義する：

```
        ══════════════════════════════════════════════════════════════
                       【 ADX HARDWARE STANDARD 】
         ・8748 Form Factor (M3ネジ / ワッシャークリアランス / 掴み幅)
         ・Unidirectional I/F Edge (単一辺へのコネクタ集約)
         ・20-Pin Expansion Bus (直スタック ＆ フラットリボンケーブル対応)
        ══════════════════════════════════════════════════════════════
                                    │
         ┌──────────────────────────┴──────────────────────────┐
         ▼                                                     ▼
【 旗艦MCUボード（主役）】                            【 拡張CARD群（周辺）】
  ADX CORE-U (The Enclosure Board)                      ADX CARD Ecosystem
  ・$20〜$25 想定                                       ・Prototyping CARD (ユニバーサル)
  ・ATtiny1616 + USB Type-C                             ・AAA 2S Battery CARD (乾電池)
  ・Arduino IDE + megaTinyCore ネイティブ               ・3Dプリントケース / マウントSTL
  ・(※電源回路等の詳細仕様は策定中/個別docs参照)                   
                                                               │
                                                               ▼ (将来のおまけ)
                                                      【 将来の派生ボード（Horizon）】
                                                        ADX CORE-A (Industrial Concept)
                                                        ・同じ20Pバス/8748規格を使った
                                                          現場用上位ボード構想（末尾に小さく言及）
```

---

## 5. ドキュメント別 改訂計画（Document Modification Matrix）

| 対象ドキュメント | 改訂の方向性・重点項目 | 優先度 |
| :--- | :--- | :---: |
| **`README.md`**<br>(ルート英語) | ・「Industrial standard」の看板を完全撤去し、**「An Enclosure-Friendly Arduino-Compatible Board」**として全面刷新。<br>・Dual Type-C（Debug + 3A Power）と理想ダイオード電源の賢さを図解で解説。<br>・3Dプリントによるエンクロージャ親和性を強調。<br>・CORE-Aは最下部の「Future Ideas」として1段落でこっそり言及。 | **最優先 (P0)** |
| **`docs/ja/README.md`**<br>(日本語) | ・ルート `README.md` の改訂内容を忠実に日本語化。<br>・過度な専門用語を排し、日本の個人メイカーにも親しみやすい言葉遣いで表現。 | **最優先 (P0)** |
| **`hardware/CORE-U/concept.md`** | ・Dual Type-C（16P Debug + 6P Power）のハードウェア仕様を明文化。<br>・LM66100 × 3個（Power側2並列 + Debug側1個、Priority連動）の電源ブロック図と設計根拠を追記。 | **高 (P1)** |
| **`docs/ja/adx_formfactor_guidelines.md`** | ・RFC 2119 ガイドラインにおける「MUST / SHOULD / MAY」の定義を再整理。<br>・産業用要件の押し付けを排除し、筐体設計におけるクリアランスや電源保護ルールに焦点を当てる。 | **高 (P1)** |
| **`memo/merketing/market_positioning.md`** | ・今回のピボット（CORE-U主軸化、r/Arduino対話戦略、ハーメルンの笛吹きコンテンツ戦略）を反映した新戦略書へ更新。 | **中 (P2)** |

---

## 6. 新 `README.md` の構成案（ワイヤーフレーム）

r/Arduinoの住人が読んだ瞬間に「これ欲しい！」「面白い！」と思える、**規格（Form Factor）と製品（CORE-U）を明確に分けた章立て構成**：

### 1. Catchphrase & Hero Visual
- **Hero Image Placeholder**:
  `![ADX CORE-U Board & Enclosure](docs/images/hero_no_photo.png)` *(※現在は「No Photo / 写真準備中」のプレースホルダーを設置)*
  - 読者が一目で「欲しい！」と感じる、ケースやマウントに美しく収まったCORE-Uの実物写真・3Dレンダリングを表示する領域。
- **Catchphrase**:
  - *“ADX CORE-U: An enclosure-friendly, dual-USB Arduino-compatible board designed for tidy wiring and hassle-free power delivery.”*
  - （すっきりとした配線とスマートな給電を実現する、筐体に優しいDual USB搭載のArduino派生ボード）

### 2. Overview: From Workbench to Enclosure (ポジティブな設計思想)
- ペイン煽りを排し、プロダクトの設計意図をポジティブかつ明快に記述：
  - *“Building prototypes on a workbench is fun, but putting them into a neat box is where real projects come to life. ADX CORE-U is designed from the ground up to make custom enclosure integration effortless, featuring standardized mounting clearances, clean edge interfaces, and an intelligent dual-USB power system.”*
  - （ブレッドボードでの試作はもちろん楽しいですが、それを綺麗な箱に収めることで作品は真の完成品になります。ADX CORE-Uは、ネジ止めや固定のしやすさ、一方向に揃った配線、そしてサーボ等に大電流を流せる安全なDual USB電源を備え、箱への組み込みをどこまでもスムーズにするために設計されました）

---

### 3. The Foundation: The ADX Form Factor & Bus Standard (規格としての性質)
特定ケースへの依存を排し、あらゆる自作ケース・既製ボックスへの組付け性を最大化する**オープン・ハードウェア規格**としての仕様：

1. **8748 Form Factor & Mechanicals (固定・保持の自由度)**:
   - `87.0 mm × 48.0 mm` 外形寸法。
   - 4隅のM3ネジ穴（`77.0 mm × 38.0 mm` ピッチ） ＆ **Φ5.0 mm ワッシャークリアランス**（ネジ締めによるパターン破壊を防止）。
   - レールへの**スロットイン**や、3Dプリント爪の**スナップフィット**に対応する基板外周の**掴み幅（エッジマージン）**。
2. **Unidirectional Interface Edge (I/F辺の明示的規格化)**:
   - コネクタの四方飛び出しを根絶し、外部接続コネクタを**単一の「I/F辺」に集約**。
   - ケースの穴あけ加工や3Dプリント開口部設計が「1面だけ」で完結。
3. **20-Pin Bus & Flexible Topology (柔軟な筐体内配置)**:
   - 5V VDD / GND / 通信ラインを束ねた統一20P拡張バス。
   - 基板同士の直スタックだけでなく、**市販のIDCフラットリボンケーブル接続を標準規格化**。
   - 薄型ケースなら「横並び」、深型ボックスなら「上下スタック」、段違いボスなら「ケーブルで逃がす」など、筐体内部の空間形状に合わせた自由自在なレイアウトが可能。
4. **The Modular CARD Ecosystem**:
   - 20Pバスで繋がる「Prototyping CARD」と「AAA Battery CARD」。

---

### 4. The Flagship Board: ADX CORE-U (個別製品としての性質)
ADX規格を体現し、Arduino IDEで手軽に動く**旗艦MCUボード**としての仕様：

1. **Seamless Arduino IDE Experience**:
   - ATtiny1616-MNR ＋ CH342K（USB Type-C）。
   - ブートローダー不要、`megaTinyCore` ネイティブ対応により、普段のArduino IDEからUSBケーブル1本で即座にスケッチを書き込み・デバッグ可能。
2. **Thoughtful Power & Protection Design (In Active Development)**:
   - ホビー工作での利便性を第一に考え、Type-C給電、PCの逆流保護、サーボ駆動などの大電流対応を現在最適化・設計中。
3. **Full Hardware Specifications & Documentation**:
   - 電源回路、ピン配置、最新のハードウェア仕様・設計ノートについては、詳細ドキュメント（`hardware/CORE-U/`）を参照。

---

### 5. Open Hardware & 3D Printing (コミュニティリソース)
- 回路図、KiCadデータ、基板製造ファイル。
- 3Dプリント用ケースやマウントブラケットのオープンSTLデータ。

### 6. Future Horizon (将来展望・おまけとして1段落だけ)
- *“Future Note: The ADX 20-pin bus is designed with robust modularity in mind. In the future, specialized heavy-duty core boards (like galvanically isolated RS-485 variants) might join the family, sharing the exact same CARDs.”*
- （将来的には、同じCARDが使える完全絶縁RS-485版などのタフな派生ボードも登場するかもしれません、程度に軽く触れる）

---

## 7. 実施手順とマイルストーン

1. **Step 1: 本計画書のFIX** (現在地)
2. **Step 2: `hardware/CORE-U/concept.md` の仕様更新**
   - Dual Type-C ＋ 3x LM66100 の電源仕様を反映。
3. **Step 3: アイキャッチ用「No Photo」画像の設置 ＆ ルート `README.md`（英語）のドラフト作成**
   - 冒頭に配置する「No Photo」プレースホルダー画像（またはSVG/アセット）を用意。
   - ペイン煽りを排した、ポジティブで誠実な英語表現による `README.md` の全面刷新。
4. **Step 4: `docs/ja/README.md`（日本語）の同期作成**
5. **Step 5: ガイドラインおよびマーケティング資料のアップデート**
