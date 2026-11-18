# Show Bible & Scriptwriting Framework (v3: DEV First-Person Monologue)
## Working Title: DEV’s Field Log / ADX Engineering Chronicles (TBD)

本ドキュメントは、オープンハードウェア規格「ADX」の実開発記録をベースにした英語技術系 Podcast / YouTube 番組の設計書（**v3：DEV 一人称モノローグ版**）です。

v1（2人掛け合い）、v2（ECHO独白）を経て辿り着いた、**英語圏のテック系メディア（*Stuff Made Here*, *Ben Eater*, *Technology Connections* 等）に最も自然に馴染む「開発者本人（`DEV`）によるダイレクトな一人称語り」**をベースに、v1/v2で培った硬質な世界観と「AI（`ECHO`）からの正論ツッコミ」を**話のネタ・引用**として組み込む構成です。

---

## 1. コア・コンセプト

> **「規格外のハックを仕掛けた開発者本人が、『AIにはこう怒られたけど、現場ではこうするしかなかった』と真っ直ぐ語る実録ログ。」**

* **形式**: **First-Person Engineering Monologue（開発者単独の一人語り）**
  * リスナーに対して「なぜ俺がこのヤバい設計を選んだのか」を、変な前置きや芝居がかった掛け合い抜きでストレートに語る。
  * 話者は **`DEV`（1人）のみ**。英語圏リスナーにとって最も聴きやすく、信頼感と没入感が高いスタイル。
* **ECHO（AI）のポジション（案B寄りハイブリッド）**:
  * **音声としては登場しない（無言の監査ボット／コンプライアンスAI）**。
  * DEVが設計差分やBOMをチェックにかけた際に、**「AI（ECHO）が真っ赤な警告を出してキレてきた内容」をDEV自身が面白おかしく引用・紹介する**形で登場させる。
  * 現代のエンジニアにとって「AIにコードや回路をレビューさせて正論で怒られる」のはリアルな日常体験でもあるため、SF世界観を保ちつつも極めて現実味のある笑いになる。
  * YouTube版では、DEVがECHOの警告に触れるタイミングで画面端に静かなHUD警告（`[ECHO // COMPLIANCE ALERT]`）を出す演出が可能。
* **音声ファースト（Podcast完全互換）**:
  * 1人の語り手だけでテンポよく完結するため、Spotify / Apple Podcasts 等の音声メディアと相性が抜群。

---

## 2. 語り手（DEV）のキャラクターとトーン

### DEV（当事者エンジニア / 一人称 `"I"`）
* **トーン**: 落ち着いていて、少しドライで現実主義。自慢するのではなく、「綺麗な設計にしたかったんだけど、現実が許してくれなかった」という**実務者の苦笑いと割り切り**で語る。
* **セリフの特徴**:
  * 1文目から結論（自分の犯したタブー）を言い切る。
  * 過剰にテンションを上げない（YouTuber的な大声や煽りは不要）。
  * 自虐と皮肉を混ぜる（*"Look, I know how that sounds."* / *"Even my own review AI asked if I was trying to commit arson."*）。

---

## 3. DEV中心・4ビート構成（The 4-Beat Field Log）

標準尺は **3〜4分（英語約450〜650語）**。以下の4ビートでドライブします。

```text
[1. The Confession]  ──▶  [2. The AI Objection]  ──▶  [3. The Ugly Reality]  ──▶  [4. The Brute-Force Fix]
   1文目でタブーを自白        ECHO/正論のツッコミを引用      綺麗な正論が死ぬ現場の制約      力技の種明かしと潔い締め
```

### Beat 1: The Confession（0:00 - 0:30｜1文目の自白）
* 挨拶や状況設定（「監査ログを開始します」等）を一切省き、自分がやった常識外の設計変更を1文目で投下する。
* *例*: *"I just ripped WebSerial out of our industrial field probe and replaced it with MIDI. Yes—the 1983 music standard."*

### Beat 2: The AI / Sanity Objection（0:30 - 1:15｜AIの警告と正論の先回り）
* 「当然、これには猛反対が来た」と切り出し、**ECHO（監査AI）が吐き出した警告や、データシート警察が叫びたくなるポイント**をDEV自身が代弁して叩く。
* **比喩翻訳ルール**: 専門用語（SysEx、LIN break、12V HV UPDI等）が出たら、DEVの口から「つまり平たく言うとこういう無茶だ」と耳でわかる比喩に噛み砕く。

### Beat 3: The Ugly Reality（1:15 - 2:15｜教科書が通用しない現場）
* 「机の上ではAIの言う通り100%正しい。でも現場は違った」と視点をひっくり返す。
* Android Chromeの仕様、酸化したOSPパッドの14%不良、納期48週の車載コネクタなど、**反論不能な物理・運用・コストの現実**を提示する。

### Beat 4: The Brute-Force Fix & Grudging Sign-off（2:15 - 3:30｜力技の解法とAIの渋々承認）
* どうやってそのリスクを物理的・アーキテクチャ的にねじ伏せたか（RP2040 PIO、全ラインESDクランプ、USB無通電仕様等）を解説する。
* 最後は「美しくはないが、計算は合っている。さすがにECHOも渋々パスさせた」というオチでスパッと終わる。

---

## 4. 台本フォーマット（`@DEV:` + コメント形式）

v1/v2と同じパーサー思想を継承し、単一話者 `@DEV:` で記述します。

```text
# Episode Title / Subtitle

# [VISUAL: ...]
# [HUD: ECHO // AUDIT WARNING: ...]
@DEV: English_spoken_line_here.
# （日本語対訳）

# [PAUSE 0.5s]
@DEV: Next_English_spoken_line_here.
# （日本語対訳）
```

### Python 抽出スクリプト例（v3対応）

```python
import re

def parse_v3_script(script_text: str):
    """Extract @DEV blocks and their immediately following Japanese translation comments."""
    pattern = r'^@DEV:\s*(.+?)\r?\n#\s*（(.+?)）'
    matches = re.findall(pattern, script_text, re.MULTILINE)
    return [{"speaker": "DEV", "en": m[0].strip(), "ja": m[1].strip()} for m in matches]

def to_tts_lines(script_text: str):
    """Return clean English lines for single-voice TTS or teleprompter."""
    lines = []
    for line in script_text.splitlines():
        stripped = line.lstrip()
        if not stripped or stripped.startswith("#"):
            continue
        if stripped.startswith("@DEV:"):
            lines.append(stripped[len("@DEV:"):].strip())
    return lines
```
