# Episode 06: Steve Jobs vs. AI Hardware—Why Craftsmanship Is Not an Optimization Problem
# Off the Breadboard — Stage 2 Production Script (Converted from drafts/ep06_stage1_craftsmanship_not_optimization.md)

# [VISUAL: Terminal log connecting an AI agent to PCB CAD — 20 minutes later, DRC 0 errors and a Gerber ZIP appears on screen]
@DEV: Today, I handed an AI agent control of my PCB CAD software. Twenty minutes later, it dodged an obsolete LED, routed the board, passed design rule checks, and exported factory-ready Gerbers.
# （今日、俺はAIエージェントにPCB CADソフトの操縦桿を渡した。20分後、AIは廃番LEDを自力で避け、配線を済ませ、DRCを通過し、工場直行のガーバーまで書き出しやがった。）

@DEV: Instead of celebrating, my finger froze over the order button—because watching that convinced me we are about to flood the planet with physical e-waste.
# （喜ぶどころか、俺の指は発注ボタンの上で凍りついた――その光景を見て、これから世界中に物理的な電子ゴミが溢れ返ると確信してしまったからだ。）

# [PAUSE 0.4s]
@DEV: I know what you're thinking. "Wait—your AI built factory board files in twenty minutes! Every datasheet and routing rule is instant and free now. How could craftsmanship still be missing in twenty twenty-six?"
# （あんたが思っていることは分かってる。「待てよ――AIが20分で工場用基板データを作ったんだろ！ あらゆるデータシートも配線ルールも一瞬でタダだぞ。2026年になっても、どうやってクラフトマンシップが欠落するって言うんだ？」ってな。）

# [PAUSE 0.4s]
# [VISUAL: Archival quote card — Steve Jobs, "Triumph of the Nerds" (1995): "The disease of thinking that a really great idea is 90% of the work."]
@DEV: Rewind to nineteen ninety-five. In his Triumph of the Nerds interview, Steve Jobs warned of a serious disease: thinking a great idea is ninety percent of the work—while forgetting the massive valley of craftsmanship between a great idea and a great product.
# （1995年に巻き戻せ。『Triumph of the Nerds』のインタビューで、スティーブ・ジョブズはある深刻な病を警告した――素晴らしいアイデアが仕事の90%だと思い込み、アイデアと偉大な製品の間に横たわる膨大なクラフトマンシップの谷を忘れる病だ。）

@DEV: Notice who Jobs blamed—not the tools. Today, AI didn't cure that ninety-percent disease. It strapped a rocket engine to it.
# （ジョブズが責めていた相手に注目しろ――道具じゃない。そして今日、AIはその90%病を治したんじゃない。ロケットエンジンをくくりつけたんだ。）

# [VISUAL: Contrast split — Software patch / server shutdown vs. a warehouse stacked with unsold physical hardware]
@DEV: Ship sloppy software, and you patch it on Tuesday—or quietly shut down the server. No warehouse rent. No landfill.
# （雑なソフトウェアを出しても、火曜日にパッチを当てるか、静かにサーバーを落とせば消える。倉庫代もかからないし、埋め立てゴミにもならない。）

@DEV: Now watch deep-pocketed software companies grab AI speed and turnkey factories and charge into physical gadgets—before ever feeling the cold sweat of unsold stock. They don't ship a buggy app. They mass-produce physical landfill.
# （そこで、潤沢な資金を持つソフトウェア企業がAIの速度と丸投げできる工場を掴み、売れ残り在庫の冷や汗を一度も知らぬまま物理ガジェットへ突っ込んだらどうなるか。バグだらけのアプリじゃない。物理的な埋め立てゴミを量産するんだよ。）

# [PAUSE 0.5s]
@DEV: Okay, you're pushing back. "Fine—software companies might burn themselves on inventory. What about veteran hardware makers? They know dead stock! Hand them AI, and they're unstoppable!"
# （なるほど、あんたは言い返すはずだ。「分かった、ソフト企業は在庫で火傷するかもしれない。だがベテランのハードウェアメーカーなら別だろ？ 連中は不良在庫を知っている！ AIを渡せば無敵じゃないか！」と。）

# [PAUSE 0.4s]
# [VISUAL: Two rival factories — Company A ships Lot 1; Company B uses AI to reverse-engineer, scrape 1-star reviews, and route around patents before Lot 1 sells out]
@DEV: Picture two equal veteran manufacturers: Company A and Company B. Company A takes the risk, pays for development, builds a big first lot, and ships Product A.
# （同じ実力を持つ2社のベテランメーカー、A社とB社を想像してみろ。A社がリスクを取り、開発費を背負い、初回の大ロットを組んで製品Aを出荷する。）

@DEV: Before Lot One even sells out, Company B feeds Product A into AI—reverse-engineering it in days, scraping every angry review, and routing around patents faster than any patent office moves. Then Company B drops improved Product B onto the exact same shelves.
# （初回ロットが捌け切る前に、B社は製品AをAIに食わせる――数日で設計を丸裸にし、不満だらけのレビューを全部吸い上げ、どこの特許庁が動くより速く特許を迂回する。そして改良品Bを、まったく同じ売り場へぶつけてくるんだ。）

@DEV: Traditional factory math worked because one dam held: product freshness outlasted your production and inventory lead time. Turn hardware into late-throw rock-paper-scissors, and big-batch cost optimization flips overnight into a landmine of dead inventory.
# （従来の工場の計算式は、たった1つの防波堤があって成立していた――製品の鮮度が、生産と在庫消化のリードタイムより長く保つという防波堤だ。ハードウェアが後出しじゃんけんに変わった瞬間、大ロットの原価最適化は、一晩で死蔵在庫の地雷原へひっくり返る。）

# [PAUSE 0.5s]
# [VISUAL: Optimization curve converging on an "80-Point Honor Student" vs. a branching evolutionary ecosystem]
@DEV: Right there is the biggest trap of the AI era. We assumed optimizing specs, cost, thermals, and reviews simultaneously would deliver craftsmanship.
# （そこにこそ、AI時代最大の罠がある。俺たちは、スペックもコストも放熱もレビューも同時に最適化していけば、クラフトマンシップに辿り着くと思い込んでいた。）

@DEV: It won't. AI is an optimization machine—hand it a scorecard, and it mass-produces safe, polite, eighty-point honor students for almost nothing. Look at nature: after hundreds of millions of years, did evolution ever converge on one optimal creature?
# （辿り着かない。AIは最適化マシンだ――採点表を渡せば、無難で礼儀正しい「80点の平均的優等生」をほぼタダで無限量産する。自然界を見てみろ――何億年もの進化を経て、自然はたった1種類の最適解の生き物に収束したか？）

@DEV: Never. Polite generalists get wiped out everywhere. Ecosystems survive through ruthless specialized diversity.
# （一度もない。器用貧乏な優等生はどこでも淘汰される。生態系は、極限まで振り切った「特化型多様性」で生き残っているんだ。）

# [PAUSE 0.4s]
# [VISUAL: Global hardware store fastener aisle — bulk commodity hex nuts vs. nylon-insert lock nuts and heavy-duty wedge-locking nuts, backed by 4 layers: Product / Tooling / Sales / Logistics]
@DEV: Walk down any hardware store fastener aisle and look at metal nuts. On one end: dirt-cheap commodity hex nuts stamped by the millions for fractions of a cent—ruthlessly specialized in brute-force volume and global cost.
# （どこのホームセンターでもいい、ネジ売り場を歩いて金属のナットを見てみろ。片方の端には、1個コンマ数セントで何百万個も量産される激安の汎用六角ナットがある――圧倒的な量産性とコストに極振りした特化型だ。）

@DEV: On the other end: nylon-insert lock nuts and heavy-duty wedge-locking nuts built so a bridge or jet engine never shakes loose. Why hasn't an AI-powered rival wiped them out overnight?
# （もう片方の端には、橋やジェットエンジンが振動で絶対に緩まないよう作り込まれた、ナイロンインサートロックナットや、くさび式の産業用ロックナットがある。なぜAIを持った競合が、一晩でこいつらを駆逐できていないんだ？）

@DEV: Because their moat isn't a CAD file AI can redraw in five minutes. It is four specialized layers locked together: product performance, factory tooling, sales relationships, and warehouses that never run dry. Copy the geometry—you still can't copy the habitat.
# （やつらの堀が、AIが5分で描き直せるCAD図面じゃないからだ。4つの特化した層が噛み合っている――製品特性、専用の製造設備、営業と信頼関係、そして絶対に欠品しない物流拠点だ。形状をコピーしても、生息環境まではコピーできないんだよ。）

# [PAUSE 0.6s]
# [VISUAL: ADX CORE U sitting between the official Arduino UNO ecosystem and ultra-cheap clone supply chains]
@DEV: That is why my finger kept freezing over the order button for ADX CORE U. Building a neat general-purpose microcontroller board with AI is completely doable—I just watched an agent export Gerbers in twenty minutes.
# （だからこそ、俺の指は ADX CORE U の発注ボタンの上で毎回止まっていたんだ。AIを使って端正な汎用マイコンボードを作ること自体は余裕でできる――現についさっき、エージェントが20分でガーバーを吐くのを見たばかりだ。）

@DEV: But building a board has nothing to do with surviving between official Arduino's brand-and-classroom ecosystem on one side, and hyper-cheap clones' volume-and-logistics ecosystem on the other. Drop an eighty-point honor student into that aisle, and nature deletes you.
# （だが、基板を作れることと、Arduino本家のブランド・教育の生態系と、激安互換機の量産・物流の生態系のあいだで生き残ることは、まったく別問題だ。あの売り場のど真ん中に80点の平均的優等生を放り込んだ瞬間、自然界はお前を消去する。）

# [PAUSE 0.5s]
# [VISUAL: Bold final line — "How to optimize?" crossed out → "How to survive?"]
@DEV: Craftsmanship is not an optimization problem. It is a survival problem. Stop asking "How do I optimize?"—start asking "How do I survive?" See you on the next episode of Off the Breadboard.
# （クラフトマンシップは最適化問題じゃない。生き残りの問題だ。「どう最適化するか？」と訊くのはやめろ――「どうやって生き残るか？」と訊け。また次回の『Off the Breadboard』で会おう。）
