# Show Bible & Scriptwriting Framework (v2: Solo Monologue)
## ECHO’s Audit Log (Working Title) — *ADX Hardware & Protocol Dissections*

> **仮タイトル**: `ECHO’s Audit Log`（変更の余地あり。候補：`ECHO’s Sanity Check` / `ECHO’s Incident Log` 等）

本ドキュメントは、オープンハードウェア規格「ADX」の開発記録を題材とした英語技術系 Podcast / YouTube 番組の設計書（v2：ECHO 単独モノローグ版）です。

v1（`contents/v1/`）の硬質なサイバーパンク世界観と「現場の暴走 vs 正論のAI」という構図は継承しつつ、英語圏の Audio Essay / Solo Podcast リスナーに自然に刺さる **「戦術観測AI（ECHO）の単独監査ログ朗読」** フォーマットへと構造を最適化しています。

---

## 1. コア・コンセプト

> **「常識外れの現場ハックを、胃を痛めたAIが解剖し、最後に悔しそうに承認する監査ログ。」**

* **ジャンル**: サイバーパンク・テック・オーディオエッセイ / ハードウェア設計監査ドキュメンタリー
* **番組形態**: **Solo Tactical Audit Log（AI単独による設計監査ブリーフィング）**
  * 2人の架空キャラクターによる台本掛け合い（英語圏で「芝居臭さ」や「AI同士の会話感」が出やすい形式）を排し、**1つの人格を持ったAI（ECHO）がリスナー（＝Command / 現場の同僚エンジニア）へ直接語りかける**。
  * TTS（合成音声）を使う場合でも、**「AIが監査ログを読み上げている」という設定自体が音声の質感と完全に一致する（Diegetic Authenticity）**ため、不自然さが出ない。
* **DEV（人間エンジニア）の扱い**:
  * **声は出さない。** 画面や音声の向こう側にいる「名前も顔も見えない現場の悪魔（the lead engineer / Dev）」として、**Gitコミット、回路図の差分、BOM、1行の雑なメモ**としてのみ登場する。
  * ECHOが *「彼のコミットメッセージは全文でたった4語だった。引用しよう――」* と読み上げることで、対話劇よりもむしろ狂気とシニカルな笑いが際立つ。
* **音声ファースト・映像疎結合（Audio-First）**:
  * Spotify / Apple Podcasts で耳だけで100%完結し、YouTubeでは KiCad・Git diff・データシート・3DモデルのB-rollを流すだけで成立する。

---

## 2. 語り手キャラクター（ECHO）の設計

### ECHO（戦術観測知性 / Callsign: ECHO）
* **ポジション**: 安全規格（W3C、IEEE、JEDEC、熱・電気安全基準）を背負った監査AI。
* **語り口（Voice & Tone）**:
  * *Portal* の GLaDOS や、深夜の管制室にいる冷静だが皮肉屋のミッションコントロールのような、**礼儀正しく怜悧でありながら、呆れと皮肉（Dry Wit / Sarcasm）が滲む語り口**。
  * 大声で叫んだりYouTuber風に煽ったりはしない。淡々と数値を読み上げ、一瞬の沈黙のあとで辛辣な比喩を刺す。
  * **聴き手との関係性**: リスナーを「監査記録の閲覧者（または Command / 他のエンジニア）」として扱い、共犯者的に愚痴を共有する（*"Stay with me here."* / *"I wish I were making this up."*）。

---

## 3. 独白版・4大黄金ルール（The 4 Cardinal Rules — Solo Edition）

すべてのエピソード（標準 **3〜5分 / 約450〜750語**）は、以下の4ステップで構成します。

### Rule 1: The Anomalous Commit（異常コミットと雑な根拠の提示）
* 冒頭30秒以内に、DEVがプッシュした「常識的にあり得ない設計変更」と、彼のそっけない残置メモ／コミット文を提示してフックを作る。
* *例*: *"Commit `decf786`. The lead engineer excised WebSerial from the field probe roadmap and replaced it with a 1983 synthesizer protocol. His entire written justification? Four words: 'WebSerial is a brick.'"*

### Rule 2: Disbelief & Physics Reality Check（正論と皮肉による解剖）
* ECHOが「なぜそれが教科書的に最悪のアイデアに見えるのか」を、安全基準・帯域損失・物理法則で叩く。
* ここで v1 同様の Geek Lexicon（*Magic Smoke*, *Commit Arson*, *Bed-of-Nails*, *Brute Force* 等）を自然に織り込む。

### Rule 3: The Metaphor Translation Bite（耳でわかる比喩翻訳）★最重要
* 専門用語（SysEx、CCL、LCP樹脂、SIGROW、PIO等）を出した直後に、必ず **「つまり平たく言うと、彼は〇〇したわけです（In plain English, he... / Essentially, he...）」** と鮮烈な日常比喩へ翻訳する。
* 画面を見ない Podcast リスナーが、頭の中で一瞬で状況とヤバさを映像化できるようにする。

### Rule 4: The Reality Pivot & Bitter Sign-Off（現実の発見と悔しい承認）
* 独白の後半、ECHOが製造ログ・OS制限・部品在庫などのデータを照合し、「正攻法が現実世界で詰んでいた」事実に気づく。
* さらに回路やコードの奥に仕込まれた「力押しの安全マージン（ESDクランプ、PIO自律制御、無通電仕様など）」の計算が完璧に合っていることを検証してしまい、**「……計算が合っているのが本当に腹立たしい（I hate that his math actually checks out.）」** と悔しがりながら `SPEC APPROVED` を押して締める。

---

## 4. 独白でダレさせないペーシング技法（Solo Pacing Rules）

独白は長文が続くと単調になりやすいため、台本執筆時に以下を徹底します。

1. **短文と断定のミックス**: 1文はなるべく20語以内に抑え、長い説明の直後に3〜6語の短いツッコミを置く。
   * *例*: *"He chose Web MIDI. A standard built in 1983 for electric keyboards. For industrial machinery."*
2. **引用フレーム（Quote Framing）**: DEVの言葉を読む前に必ず区切りを入れる。
   * *"And I quote:"* / *"His note reads:"* / *"To borrow his exact words:"*
3. **リアルタイム解析演出（Live Telemetry）**: 「今この場でシミュレーション／ログ照合を走らせている」間を入れることで、1人語りの中に時間経過と逆転のドラマを作る。
   * *例*: *"Pulling browser compatibility matrix... Wait. ...Checking Android Chrome policies."*
4. **無音とSEのフック**: 感情の転換点（呆れ → 解析 → 悔しい承認）には `# [PAUSE 0.8s]` や `# [SFX: ...]` のコメントを置き、編集・TTS生成時の間合いに使う。

---

## 5. 台本フォーマット（`@ECHO:` + コメント形式）

TTSパイプラインへの自動投入と、編集者による日本語確認・SE配置を両立するため、以下の形式で記述します。

```text
# Episode Title / Subtitle

# [SFX: low_terminal_hum_start]
@ECHO: English_narration_sentence_here.
# （日本語対訳）

# [PAUSE 0.6s]
@ECHO: Next_English_sentence_here.
# （日本語対訳）
```

* **`@ECHO:`**: TTSに送る英語セリフ（1ブロック＝1〜3文程度の自然な息継ぎ単位）。
* **`# ...`**: コメント行。日本語対訳や、`# [PAUSE 0.6s]` / `# [SFX: ...]` / `# [VISUAL: ...]` といった制作メタ情報を記載。`#` 行を除外するだけで純粋な英語TTS用テキストになります。

### Python 抽出スクリプト例（v2対応）

```python
import re

def parse_v2_script(script_text: str):
    """Extract @ECHO blocks and their immediately following Japanese translation comments."""
    pattern = r'^@ECHO:\s*(.+?)\r?\n#\s*（(.+?)）'
    matches = re.findall(pattern, script_text, re.MULTILINE)
    return [{"speaker": "ECHO", "en": m[0].strip(), "ja": m[1].strip()} for m in matches]

def to_tts_lines(script_text: str):
    """Return clean English lines for TTS (strips all # comment lines and @ECHO: prefix)."""
    lines = []
    for line in script_text.splitlines():
        stripped = line.lstrip()
        if not stripped or stripped.startswith("#"):
            continue
        if stripped.startswith("@ECHO:"):
            lines.append(stripped[len("@ECHO:"):].strip())
    return lines
```

---

## 6. 制作フロー（1エピソード完結）

1. **題材選定**: Gitログ・設計変更メモ・検証ログから「常識破りだが現場では合理的だった判断」を1つ選ぶ。
2. **台本作成**: 上記4大黄金ルールに沿って `episodes/epXX_<slug>.md` を執筆。
3. **音声生成**: `to_tts_lines()` で抽出した英文を単一ボイス（ECHO）で生成し、指定ポーズとわずかな無線／端末エフェクト（SE）を乗せる。
4. **公開**:
   * **Podcast**: 音声をそのまま配信。
   * **YouTube**: KiCad回路図・Git diff・データシートのパン/ズーム映像＋シンプルなHUDオーバーレイを合わせて書き出し。
