# Show Bible & Production Pipeline (v5: 2-Stage Dialectical-to-Direct Pipeline)
## Working Title: ADX Engineering Chronicles / Built for the Box (TBD)

**v5** は、表現形式（フォーマット）の刷新ではなく、**「面白さと論理性・ストーリー構成を安定して量産するための制作パイプライン」**の定義です。

v1〜v4の検証で判明した以下の2つの事実を統合します：
1. **発想・ドラマ作りの最強エンジンは `v1`（DEV vs ECHO 掛け合い）である**
   * 最初から一人語り（v4）を書こうとすると、単なる「変更点の技術説明（Changelog）」になりがちで、どこが狂気なのか・どの比喩が刺さるのかを見落としやすい。
   * 一度 `DEV` と `ECHO` に殴り合いをさせることで、**①禁忌フック、②正論ツッコミ、③耳でわかる比喩、④数学的逆転オチ** が自動的に抽出される。
2. **英語圏での最終アウトプットの最強フォーマットは `v4`（`"I"`-to-`"You"` 直接語り）である**
   * `v1` のキャラクター劇をそのまま出さず、ECHOの役割を **「聴き手（`"You"`）」** へ溶かし込むことで、日本の照れ隠し感・AI寸劇感をゼロにし、英語ネイティブなテック講演／Podcastへと昇華させる。

---

## 1. v5 制作パイプライン（Stage 1 → Stage 2）

```text
  [ Git Commit / 開発メモ / 設計ジレンマ ]
                    │
                    ▼
┌──────────────────────────────────────────────────────────┐
│  Stage 1: Adversarial Dialectic（v1 対話エンジン）       │
│  ・DEV と ECHO を戦わせて「物語の骨格」を強制抽出する     │
│  ・出力先: drafts/epXX_stage1.md（内部用・公開しない）    │
└──────────────────────────────────────────────────────────┘
                    │
         【 5つの変換ルール（Conversion Rules） 】
                    │
                    ▼
┌──────────────────────────────────────────────────────────┐
│  Stage 2: Direct "I"-to-"You" Address（v4 演説台本）     │
│  ・ECHOを消去し、ツッコミと比喩を聴き手との対話へ統合     │
│  ・出力先: episodes/epXX_<slug>.md（収録・TTS用本番台本）│
└──────────────────────────────────────────────────────────┘
```

---

## 2. Stage 1（v1エンジン）で必ず抽出する4要素

Stage 1 では、完成台本としてそのまま使うことを気にせず、思い切り大げさに `DEV` と `ECHO` を衝突させます。

1. **The Taboo（DEVの禁忌）**: 一見すると「正気か？」「燃えるぞ？」と言いたくなる設計判断。
2. **The Sarcastic Attacks（ECHOの猛攻）**: データシート・規格・安全基準の観点からの容赦ない反論（2〜3個）。
3. **The Metaphor Bite（比喩翻訳）**: ECHOが叫ぶ、低レイヤ用語の鮮烈な日常比喩（例：*miniature lightning bolt*, *medieval torture device*, *bomb shelter and riot shield*）。
4. **The Math Payoff（反論不能な数字）**: 現場の現実（不良率14%、48週納期、16KBフラッシュ）と、安全マージンの実測値（3.8ns、10mA以下、1.2秒遅延）。

---

## 3. Stage 1 → Stage 2 変換の5大ルール（Conversion Matrix）

Stage 1 の対話（`@DEV` / `@ECHO`）を、Stage 2 の一人称演説（`@DEV` のみ）へ変換する機械的ルールです。

| # | Stage 1（v1 対話素材） | Stage 2（v4 直接語り）への変換ルール |
| :---: | :--- | :--- |
| **1** | `ECHO: "Session opened / Commit pulled..."`<br>（AIの起動・状況確認） | **全カット**。DEVの「1文目の自白」からいきなり始め、直後に聴き手へ振る（*"Sounds crazy, right?"*）。 |
| **2** | `ECHO` の正論・反論・怒り | **聴き手（`"You"`）の疑問** として受け止める（*"If you've read the datasheet, you're probably screaming right now..."*）。 |
| **3** | `ECHO` の現場ログ照会<br>（*"Analyzing logs... 14% failure..."*） | **聴き手を現場に立たせる描写** に変える（*"Picture yourself on the factory floor..."* / *"Here is what actually happened..."*）。 |
| **4** | `ECHO` の「比喩翻訳ツッコミ」<br>（*"So you armed every MCU with a bomb shelter?!"*） | **`DEV` 本人のまとめセリフ** としてそのまま残す（*"Think about what that means: instead of a gentle bus, I gave every chip a bomb shelter..."*）。**←これが面白さの核になる** |
| **5** | `ECHO` の「悔しい承認」<br>（*"I hate that your math checks out."*） | **普遍的な設計哲学の結び** へ昇華させる（*"It's brute force. It's ugly on paper. But brute force that works is called engineering."*）。 |

---

## 4. ディレクトリ運用

```text
v5/
├── show_bible.md     # 本ドキュメント（2段階生成フレームワーク）
├── drafts/           # Stage 1: v1対話ドラフト（ネタ出し・ドラマ抽出用）
└── episodes/         # Stage 2: v4形式の本番台本（@DEV: + # （日本語対訳））
```

### 台本フォーマット（Stage 2 `episodes/`）
`v4` と完全同一の `@DEV:` ＋ `# （日本語対訳）` 形式を使用します。

```python
import re

def parse_v5_episode(script_text: str):
    """Extract @DEV blocks and their Japanese translation comments."""
    pattern = r'^@DEV:\s*(.+?)\r?\n#\s*（(.+?)）'
    matches = re.findall(pattern, script_text, re.MULTILINE)
    return [{"speaker": "DEV", "en": m[0].strip(), "ja": m[1].strip()} for m in matches]

def to_tts_lines(script_text: str):
    """Return clean English lines for TTS or teleprompter."""
    return [
        line.lstrip()[len("@DEV:"):].strip()
        for line in script_text.splitlines()
        if line.lstrip().startswith("@DEV:")
    ]
```
