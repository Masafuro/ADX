# Episode 07: Steve Jobs' Last Question for AI Hardware—When an AI Design Review Broke Me
# Off the Breadboard — Stage 2 Production Script (Converted from drafts/ep07_stage1_ai_design_review.md)

# [VISUAL: Terminal chat log showing a brutal AI design review — followed by a tiny git diff between ADX CORE U revision four and revision five]
@DEV: Today, I asked an AI for the most brutal, objective design review of my entire hardware project. It finished in about three minutes—and for the next two hours, my brain completely shut down. I couldn't even choose a single chip on my own schematic.
# （今日、俺はAIに、自分のハードウェアプロジェクト全体に対する最も容赦なく客観的なデザインレビューを頼んだ。AIのレビューは3分ほどで終わった――そしてその後2時間の間、俺の脳は完全に機能しなくなった。自分の回路図のチップ部品1個すら選べなくなっていたんだ。）

@DEV: Yet when the dust settled and I opened the diff between ADX CORE U revision four and revision five, the board had barely changed at all.
# （なのに嵐が収まって ADX CORE U の rev4 と rev5 の差分を開いたら、基板自体はほんのわずかしか変わっていなかったんだ。）

# [PAUSE 0.4s]
@DEV: I know what you're thinking. "Wait—you literally asked for a brutal critique! Yesterday AI exported your Gerbers in twenty minutes, and today one tough review freezes your brain?"
# （あんたが思っていることは分かってる。「待てよ――自分で容赦ない批判を頼んだんだろ！ 昨日はAIが20分でガーバーを吐いたって騒いでいたのに、厳しいレビューを1回食らっただけでフリーズするのか？」ってな。）

# [PAUSE 0.4s]
# [VISUAL: Split contrast — Human engineer frowning and pausing vs. a poker-faced AI quietly generating a 200-line spec]
@DEV: Here is what actually shook me. The AI didn't turn into a villain—it never changed its expression at all. With a human engineer, if your project drifts toward a cliff, you feel it before anyone speaks: a frown, a sigh, or that half-second pause before they say "sure."
# （本当にゾッとしたのはそこなんだ。AIが悪役に豹変したんじゃない――顔色を1ミリも変えなかったんだよ。人間のエンジニアとなら、プロジェクトが崖へ向かって逸れ始めたとき、言葉になる前に気づく――眉のひそめ、ため息、「ええ、まあ」と言う前の0.5秒の溜めにな。）

@DEV: AI gives you zero of that. It quietly cranks out two-hundred-line specs and schematics with a straight face—until you explicitly say, "Now judge whether this whole project is crazy." And the instant you do, it dismantles your entire foundation with surgical precision, like it saw every flaw from day one.
# （だがAIにはその兆候がゼロだ。涼しい顔で200行の仕様書も回路図も粛々と量産し続ける――こちらが明示的に「今、このプロジェクト全体がおかしくないか評価しろ」と命じるその瞬間までな。そして命じた途端、1日目から全欠陥を見抜いていたかのような外科手術級の切れ味で、土台ごと粉砕してくるんだ。）

# [PAUSE 0.5s]
# [VISUAL: A tutor pointing at a notebook — "Look here. Write this. Listen." — next to an AI spotlight moving across a finite text space]
@DEV: Did it know all along and hide it? No. That's when an old memory clicked. Years ago, I tutored sharp elementary kids for tough entrance exams. Those kids are shockingly bright, but their attention drifts everywhere—so at every beat, you physically point and direct them: "Look here. Write this. Listen."
# （最初から気づいて黙っていたのか？ いや、違う。そこで昔の記憶が繋がった。昔、中学受験に挑む賢い小学生を塾講師として教えていたことがある。あの年代の子は驚くほど頭が切れるが、すぐ注意が散る――だから授業の要所で指を差して明確に導く必要がある。「ここを見る」「書く」「聞く」ってな。）

@DEV: AI works the exact same way. It feels omniscient, but it is just racing through a finite space of words, wherever you aim its spotlight. It isn't blind to your project's flaws—it simply never steps into the critical-audit room unless the human points and says, "Look right here."
# （AIもまったく同じだ。万能に見えるが、実態は文字列という有限な空間の中を、人間がスポットライトを向けた方向へ光速で走っているだけ。異常に気づけないんじゃない――人間が明示的に「今ここを見ろ」と指差さない限り、「批判的評価」の部屋へ足を踏み入れないだけなんだ。）

# [PAUSE 0.5s]
@DEV: Okay, you're pushing back. "Great! You aimed the spotlight and got an honest teardown—isn't that a win? Fix the bugs and move on! Why freeze over one chip?"
# （なるほど、あんたは言い返すはずだ。「最高じゃないか！ スポットライトを向けて正直な解体診断をもらえたんだろ――大勝利だろ！ 欠陥を直して進めよ、なんでチップ1個で固まるんだ？」と。）

# [PAUSE 0.4s]
# [VISUAL: Empty construction lot — thin original spark buried under compromises and bloated justifications, stripped down to bare ground]
@DEV: Because the AI was one hundred percent right. My body, my brain, and my time are finite, and I can't see the future. Honestly, my reason for turning a microcontroller board into a product was razor-thin from day one. I wished somebody else would build the board I wanted. Nobody did—so I started building ADX myself.
# （AIが100%正しかったからだ。俺の身体も脳も時間も有限で、未来も見えない。正直に言えば、マイコンボードをわざわざ製品化する理由なんて1日目からカミソリの刃みたいに細かった。誰かが俺の欲しい基板を作ってくれればよかったのに、誰も作ってくれなかった――だから自分でADXを作り始めたんだ。）

@DEV: And here is the builder's trap: reality forces compromises, yet the act of building gets genuinely fun—so your brain whispers that what you're doing must carry some grand meaning. Then a tireless machine strips that illusion away with pure logic, leaving you in an empty lot, unable to justify even one resistor. Pure logic doesn't rescue a project. It just clears the land.
# （そしてここに作り手の罠がある――現実に揉まれて妥協する一方で作ること自体が楽しくなり、脳が「今やっていることには壮大な意味があるはずだ」と囁き始める。そこへ疲れを知らない機械が純粋な正論でその幻想を剥ぎ取り、抵抗1本の選定理由すら見えない更地のど真ん中に放り出すんだ。正論はプロジェクトを救わない。ただ更地にするだけだ。）

# [PAUSE 0.6s]
# [VISUAL: Archival quote card — Steve Jobs, Stanford (2005): "If today were the last day of my life, would I want to do what I am about to do today?"]
@DEV: After two hours of staring at that rubble, one line came back to me—Steve Jobs in two thousand five: "If today were the last day of my life, would I want to do what I am about to do today?"
# （その焼け跡を2時間見つめたあと、ひとつの言葉が戻ってきた――2005年のスティーブ・ジョブズの一節だ。「もし今日が人生最後の日だとしたら、今日これからやろうとしていることを、自分は本当にやりたいだろうか？」）

@DEV: Yes, I hear you groaning—"Steve Jobs again? Last episode was nineteen ninety-five, and now Stanford?" Hear me out. Yesterday was about the outside world—why AI can't optimize market craftsmanship. Today is about the one constant left inside the human holding the soldering iron.
# （ああ、あんたの呆れた声は聞こえてるぞ――「またスティーブ・ジョブズか？ 前回は1995年で、今度はスタンフォードか？」ってな。最後まで聞いてくれ。昨日は外の世界の話だ――なぜAIは市場のクラフトマンシップを最適化できないのか。今日は、はんだごてを握る人間の内側に残された、たったひとつの定数の話だ。）

@DEV: No matter how wildly AI expands what you could build, one physical law refuses to scale: you only get one life.
# （AIが「作れるもの」の可能性をどれだけ無限に広げようと、たったひとつだけ絶対にスケールしない物理法則がある――お前の人生は1回しかないってことだ。）

# [PAUSE 0.5s]
# [VISUAL: ADX CORE U revision five rising from the cleared lot — Enclosure-Friendly / Heat-Proof / Observable core]
@DEV: I don't know the meaning of the universe, or how far ADX will go. Yet the moment I asked that question on top of the rubble, ADX CORE U revision five snapped right back together.
# （宇宙の意味も、ADXがどこまで行けるかも分からない。だが更地の上であの問いを立て直した瞬間、ADX CORE U rev5 の骨子がすっと組み上がったんだ。）

@DEV: Maybe we already know what we want most at the very beginning—before compromises, before we get drunk on the fun of building. The AI's review burned off every bloated excuse, leaving only the clean core I wanted all along. Barely changed on paper—completely rebuilt underneath.
# （もしかすると俺たちは、一番最初に「自分が一番欲しいもの」が何か、すでに分かっていたのかもしれない――妥協する前、作ることの楽しさに酔う前にな。AIの批判が肥大化した言い訳を全部焼き払い、最初から欲しかった剥き出しの核だけが残った。図面の差分はわずかだが、土台は丸ごと組み直されたんだよ。）

# [PAUSE 0.5s]
# [VISUAL: Bold closing card — "20 minutes to lay out a PCB. 20 seconds to demolish a plan. 0 tokens for your one life."]
@DEV: AI can lay out your PCB in twenty minutes, and logically demolish your business plan in twenty seconds. The one variable forever outside its token space is whether your single life is worth spending on it today.
# （AIはお前の基板を20分でレイアウトできるし、20秒で事業計画ごと論理的に更地にすることもできる。だが、あいつのトークン空間の外側に永遠に残る唯一の変数は――お前のたった1回の人生の今日を、そこに使う価値があるかどうかだ。）

@DEV: The stronger AI gets, the easier it is to mistake momentum for meaning. So before you open CAD, before you prompt an agent, plant the last banner: "If today were the last day of my life, would I want to do what I am about to do today?" See you on the next episode of Off the Breadboard.
# （AIが強くなればなるほど、勢いを「意味」と錯覚しやすくなる。だからCADを開く前に、エージェントにプロンプトを打つ前に、最後の御旗を掲げろ。「もし今日が人生最後の日だとしたら、今日これからやろうとしていることを本当にやりたいか？」――また次回の『Off the Breadboard』で会おう。）
