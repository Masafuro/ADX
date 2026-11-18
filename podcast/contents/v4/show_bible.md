# Show Bible & Scriptwriting Framework (v4: Direct "I"-to-"You" Address)
## Working Title: ADX Field Notes / Built for the Box (TBD)

本ドキュメントは、オープンハードウェア規格「ADX」の実開発記録を題材とした英語技術系 Podcast / YouTube 番組の設計書（**v4：聴き手と1対1で向き合う直接語り・演説型**）です。

v1（2人掛け合い）→ v2（ECHO独白）→ v3（DEV独白＋ECHO引用）の検証を通じて見えてきた、**「間に架空のAI（ECHO）を挟むと、DEVとECHOの閉じた会話になり、聴き手が置いてきぼりになる」**という課題を解消しました。

v4では、**架空キャラクター設定を完全に撤廃**し、英語圏の優れたテック講演・オーディオエッセイ（*Technology Connections*, *Ben Eater*, *Stuff Made Here*, カンファレンス・ライトニングトーク等）が持つ**「語り手（`"I"`）が、聴き手（`"You"`）を目の前にして直接語りかける演説的対話」**を採用します。

---

## 1. コア・コンセプト

> **「架空のキャラや前置きをすべて外し、エンジニアが聴き手の目を真っ直ぐ見て『なぜこの無茶な設計を選んだか』を語り切る。」**

* **形式**: **Direct `"I"`-to-`"You"` Engineering Talk（演説・直接対話型モノローグ）**
  * **ツッコミ役はAIではなく「聴き手（`"You"`）」自身**。
  * 「AIにこう言われた」と第三者の話をするのではなく、**「今これを聴いているあんたも、こう思ってるはずだよね？」**と聴き手の頭の中の疑問を直接拾い上げて打ち返すことで、リスナーを議論の当事者に引き込む。
* **トーン＆リズム（英語の演説的ケイデンス）**:
  * **対話の呼びかけ**: *"Picture this."*, *"Now, you're probably thinking..."*, *"Be honest:"*
  * **3拍子（Rule of Three）**: 英語で最も耳に残るリズム（*"No drivers. No polyfills. No browser updates bricking your tool."*）
  * **対比（Contrast）**: 短い2文の対比で切れ味を出す（*"On paper, it's clean. On a ladder, it's dead."*）
  * **普遍的な結び（The Takeaway）**: 最後に「この1件のハック」から一歩引いた設計哲学を聴き手に渡して締める。
* **本人朗読にもTTSにも両対応**:
  * 架空キャラの演技が不要なため、開発者本人がそのままマイクの前で（テレプロンプター的に）語るのにも、自然な語り口調のTTSで生成するのにも最適です。

---

## 2. 4ステップ構成（The 4-Stage Engineering Address）

標準尺は **3〜4分（英語約450〜600語）**。聴き手を置いてきぼりにしないため、各ステップで必ず `"You"` との接点を作ります。

### Stage 1: The Provocation（0:00 - 0:30｜聴き手への挑戦的なつかみ）
* 「自分が犯した常識破りの決断」を宣言し、直後に聴き手へボールを投げる。
* *例*: *"I threw out WebSerial and flashed industrial hardware over a 1983 synthesizer protocol. Sounds like a joke, right? Give me three minutes—because by the end, you might actually do the same thing."*

### Stage 2: Naming Your Skepticism（0:30 - 1:15｜聴き手のツッコミを代弁する）
* 「あんたが今頭の中で言いたい反論は全部わかってる」と、データシート上の正論を聴き手の言葉として先回りし、共感と信頼を得る。
* ここで専門用語を**耳だけで映像が浮かぶ平易な言葉**に噛み砕く。

### Stage 3: Putting You in the Field（1:15 - 2:15｜聴き手を現場に立たせる）
* *"Picture this."* / *"Imagine you're..."* で、聴き手自身を工場のハシゴの上や量産ラインの現場に立たせ、綺麗な教科書スペックがなぜ無力なのかを体感させる。

### Stage 4: The Mechanics & The Principle（2:15 - 3:30｜仕組みの種明かしと演説の結び）
* 実際にどうやって物理・プロトコルの壁をねじ伏せたか（RP2040 PIO、全ライン保護、LCPの耐熱構造など）を鮮やかに解き明かし、最後は**「良いエンジニアリングとは何か」という1行のテーゼ**で聴き手に語りかけて締める。

---

## 3. v1〜v4 の進化まとめ

| バージョン | 語り手 | ツッコミ役 | 特徴・位置づけ |
| :--- | :--- | :--- | :--- |
| **v1** | DEV & ECHO | ECHO | 2人掛け合い劇（日本的「ゆっくり解説」型） |
| **v2** | ECHO | ECHO | 監査AI単独ログ（SF色が強い） |
| **v3** | DEV | ECHO（引用） | DEV一人称だが、AIとの内輪話になりやすい |
| **v4** | **DEV（本人）** | **聴き手（`"You"`）** | **純粋な `"I"`-to-`"You"` 演説・対話型。架空設定なし** |

---

## 4. 台本フォーマット（`@DEV:` + コメント形式）

v3と完全互換の `@DEV:` ＋ `# （日本語対訳）` 形式を使用します。

```text
# Episode Title / Subtitle

# [VISUAL: ...]
@DEV: English_spoken_line_here.
# （日本語対訳）

# [PAUSE 0.5s]
@DEV: Next_English_spoken_line_here.
# （日本語対訳）
```

### Python 抽出スクリプト例

```python
import re

def parse_v4_script(script_text: str):
    """Extract @DEV blocks and their immediately following Japanese translation comments."""
    pattern = r'^@DEV:\s*(.+?)\r?\n#\s*（(.+?)）'
    matches = re.findall(pattern, script_text, re.MULTILINE)
    return [{"speaker": "DEV", "en": m[0].strip(), "ja": m[1].strip()} for m in matches]

def to_tts_lines(script_text: str):
    """Return clean English lines for single-voice TTS or spoken teleprompter."""
    lines = []
    for line in script_text.splitlines():
        stripped = line.lstrip()
        if not stripped or stripped.startswith("#"):
            continue
        if stripped.startswith("@DEV:"):
            lines.append(stripped[len("@DEV:"):].strip())
    return lines
```
