# Episode 03: Why 1mm Broke Our PCB—And Sony's Walkman Rule on Hardware Size
# Off the Breadboard — Stage 2 Production Script (Converted from drafts/ep03_stage1_one_millimeter.md)

# [VISUAL: Git diff highlighting Y = 48.0 mm -> 47.0 mm on the 20-pin IDC bus spec]
@DEV: This week, I moved a single connector on our circuit board by one millimeter—and blew up our entire hardware standard.
# （今週、俺は基板上のコネクタをたった1個、1ミリ動かした――そして俺たちのハードウェア規格を丸ごと爆破した。）

@DEV: One millimeter. About the thickness of a credit card. From forty-eight millimeters down to forty-seven.
# （たった1ミリ。クレジットカード1枚分くらいの厚みだ。48ミリから47ミリへ。）

# [PAUSE 0.4s]
@DEV: If you've ever maintained a hardware project, you're probably screaming at me right now. "Are you kidding me?! Legacy boards, CAD enclosures, silkscreen coordinates, sixteen spec docs—all broken over ONE millimeter?! Ten millimeters is a redesign. One millimeter is a rounding error! Why didn't you just put it at forty-seven from day one?!"
# （ハードウェア開発を少しでもやったことがあるあんたなら、今すぐ叫んでいるはずだ。「ふざけるなよ！ 過去の基板、筐体CAD、シルク座標、16本の仕様書――全部がたった1ミリで壊れたのか？！ 10ミリなら再設計だ。1ミリなんて誤差だろ！ なんで最初から47ミリにしとかなかったんだよ！」ってな。）

# [PAUSE 0.4s]
# [VISUAL: Top edge of 8854 PCB showing SMD 20-pin IDC fillets dangerously close to the V-cut scoring line]
@DEV: I hear you. In software, changing a number by one is usually noise. In hardware, one millimeter separates two completely different worlds.
# （よく分かるよ。ソフトウェアなら数字が1ズレても大抵は誤差だ。だがハードウェアでは、たった1ミリが二つの全く異なる世界を分けるんだ。）

@DEV: At forty-eight millimeters—six millimeters from the top edge—through-hole box headers and ejector headers fit fine. Everything looked clean, until we switched to full surface-mount assembly.
# （48ミリ――基板の上端から6ミリの位置でも、スルーホールのボックスヘッダーやイジェクトヘッダーは普通に載った。全部品の表面実装へ切り替えるまでは、何の問題もなく見えたんだ。）

@DEV: Drop an SMD twenty-pin header onto forty-eight, and its outer solder fillets sit right in the path of the factory's V-cut saw. One rough depaneling snap, and that blade is practically performing open-heart surgery on your solder joints.
# （48ミリにSMDの20ピン・ヘッダーを置いた瞬間、外側のハンダフィレットが工場のVカット刃の通り道に突っ込む。パネル分割でちょっと荒く折れば、あの刃がハンダ接合部に開胸手術を施す寸前になるんだ。）

@DEV: Shift it inward by one millimeter—to forty-seven—and you keep full compatibility with through-hole and ejector headers, while unlocking automated SMD assembly. One millimeter rewrites the catalog of parts you're allowed to use.
# （それをたった1ミリ内側の47ミリへずらしただけで、スルーホールとイジェクトヘッダーの完全互換を残したまま、自動実装のSMDが解禁される。たった1ミリで、使っていい部品のカタログが丸ごと書き換わるんだよ。）

# [PAUSE 0.5s]
@DEV: Now I know what you're yelling next. "That makes it WORSE! If forty-seven works for everything, why didn't you leave generous margin from the start?! Put it at forty-five—or forty-two—instead of creeping around millimeter by millimeter!"
# （次にあんたが叫ぶことも分かってる。「余計にタチが悪いぞ！ 47ミリで全部通るなら、なんで最初からたっぷり余裕を取っておかなかったんだ？！ 1ミリずつ這いずり回らずに、最初から45ミリか42ミリに置けよ！」ってな。）

# [PAUSE 0.4s]
# [VISUAL: A blank CAD screen with a simple ruler stretching from 6 inches to 24 inches]
@DEV: Alright. Since picking dimensions sounds so easy, try this right now: I hand you a blank CAD screen and tell you to design a ruler.
# （いいだろう。寸法決めがそんなに簡単だと思うなら、今ここで試してみよう。真っ白なCAD画面を渡して、あんたに「定規を1本設計してくれ」と言ったとする。）

@DEV: How long? Six inches? Twelve? Twenty-four? Even between four inches and forty, how many possible lengths exist?
# （何インチにする？ 6インチか？ 12インチか？ 24インチか？ 仮に4インチから40インチの間に絞っても、何通りの長さがある？）

@DEV: Infinite. Software gives you booleans—true or false. Hardware hands you a continuous line from zero to infinity, down to the micron, and CAD never throws a compiler error when you pick a fatal number. Infinite freedom hands you infinite rope to hang yourself.
# （無限だ。ソフトウェアはブーリアン――trueかfalseをくれる。だがハードウェアはゼロから無限大までの連続線をミクロン単位で突きつけてきて、致命的な数字を打ってもCADはコンパイルエラーを1行も吐かない。無限の自由は、自分で首を吊るための無限のロープを渡してくるんだ。）

# [PAUSE 0.4s]
# [VISUAL: Toolbox fitment drop (100 -> 20), postal mail slot gauge, and a woodworker's cut list / PCB panel layout]
@DEV: Watch what happens when you "just add a little extra margin." Stretch that ruler by five millimeters—barely a quarter of an inch. Suddenly, instead of fitting inside a hundred commercial toolboxes and cases, it only fits inside twenty. Make it a hair wider, and it fails a cheap postal slot—your shipping cost doubles overnight.
# （「ちょっと余裕を足すだけ」で何が起きるか見てみろ。その定規をたった5ミリ――4分の1インチ弱――伸ばすだけで、市販の100種類の工具箱やケースに入っていたものが、突然20種類にしか入らなくなる。ほんの少し幅を広げれば安い郵便規格から弾かれ、送料は一晩で2倍に跳ね上がる。）

@DEV: And then the factory walks in. Think of a woodworker's cut list—how many rulers can you slice out of one standard one-meter bar? In PCB manufacturing, that's panelization. Get greedy by two millimeters, and a six-board factory panel drops to four, turning economical fiberglass into expensive confetti. Every extra millimeter has a body count.
# （しかもそこに工場が乗り込んでくる。木工職人の「木取図」を想像してみろ――1メートルの標準資材から何本の定規を切り出せるか？ PCB製造ではそれがパネル面付けになる。2ミリ欲張った瞬間に、6枚取りの工場パネルは4枚取りへ転落し、経済的なガラエポが高価な紙吹雪に変わる。余計な1ミリには、必ず犠牲者が出るんだ。）

# [PAUSE 0.5s]
# [VISUAL: 100x100 mm DIN-rail PLC bay squeezing down to the 88x54 mm board, 66 mm front I/O edge, and Y=47 mm trench]
@DEV: So how did we actually land on our board size—eighty-eight by fifty-four millimeters—with a sixty-six-millimeter front I/O edge, and a connector sitting at forty-seven?
# （じゃあ俺たちはどうやって、88×54ミリという基板サイズと、66ミリの前面I/Oエッジ、そして47ミリというコネクタ位置に辿り着いたのか？）

@DEV: We didn't pick those numbers because we felt creative. We walked into a dark alley and got crushed from all four sides until only one number survived.
# （クリエイティブな気分で選んだんじゃない。暗い路地裏に踏み込んで、四方から現実の壁に押し潰され、最後に1つだけ生き残った数字を受け入れただけだ。）

@DEV: On the outside, an industrial DIN-rail PLC cabinet lives in a one-hundred-by-one-hundred-millimeter box. Subtract enclosure walls, mounting bosses, and slide rails—and you're squeezed straight down to eighty-eight by fifty-four.
# （外側の壁は、工場のDINレールPLC筐体が生きる100×100ミリの世界だ。そこから筐体の肉厚、ネジボス、スライドレールを引けば、否応なく88×54ミリまで圧縮される。）

@DEV: On the inside, that sixty-six-millimeter front I/O edge is a bare-knuckle fight over how many Ethernet RJ45 jacks, industrial spring-clamp terminals, or a chunky D-Sub nine serial connector can physically clear the corner screws.
# （内側の壁は、前面66ミリのI/Oエッジだ。EthernetのRJ45ジャックが何個並ぶか、産業用スプリングクランプ端子が何ピン収まるか、ゴツいD-Sub 9ピン端子が四隅のネジをかわして入るかという、素手の殴り合いの結果だ。）

@DEV: That's why I couldn't just shove the header down to forty-five or forty-two. Go up to forty-eight, and the V-cut saw eats your SMD fillets. Drop down to forty-two, and you slam into the routing zone and front connectors. Forty-seven isn't a guess—it's the one-millimeter trench where nothing dies.
# （だからこそ、ヘッダーを45ミリや42ミリへ適当に押し下げるわけにはいかなかったんだ。48ミリに上げればVカット刃がSMDフィレットを食い千切る。42ミリに下げれば配線エリアと前面コネクタに正面衝突する。47ミリは勘じゃない――何も死なずに済む、たった1ミリの塹壕なんだよ。）

# [PAUSE 0.6s]
@DEV: On a schematic, one millimeter looks like petty obsession. In reality, it's a multi-car pileup of saws, cabinets, connectors, toolboxes, and cut lists.
# （回路図の上では、たった1ミリなんて神経質なこだわりに見える。だが現実には、ノコギリ、筐体、コネクタ、工具箱、木取図が絡み合う玉突き事故なんだ。）

# [VISUAL: Sony Walkman Rule #3 — "Decide size and cost by necessity, not possibility"]
@DEV: Years ago, the Sony engineering team behind the original Walkman had an internal mantra—rule three of their eighteen rules of hardware development: never decide size or cost by what's possible. Decide them by necessity.
# （昔、初代ウォークマンを生み出したソニーのエンジニアチームには、内輪の鉄則――「開発18か条」の第3条――があった。「サイズやコストは『できるかどうか』の可能性で決めるな。必要性・必然性で決めろ」と。）

@DEV: On a breadboard, that sounds like a poster on an office wall. Between a V-cut saw, a DIN-rail cabinet, and sixty-six millimeters of connectors, you realize it isn't poetry—it's survival. Let necessity corner you into the number.
# （ブレッドボードの上で聞くと、オフィスの壁に貼る標語みたいに聞こえる。だが、Vカット刃とDINレール筐体と66ミリのコネクタに挟まれた瞬間、それがポエムじゃないと気づく――生存戦略なんだよ。必然性に追い詰められて数字を決めろ。）

# [PAUSE 0.5s]
# [VISUAL: ADX Reference Card v0 updated to Y=47.0 mm — and a cursor hovering over CAD]
@DEV: So we updated the specs, updated the ADX Reference Card, and locked the connector at forty-seven millimeters. Permanently frozen. Never moving another millimeter again.
# （というわけで――仕様書を更新し、ADX Reference Card も更新して、コネクタを47ミリにロックした。永久凍結だ。もう二度と1ミリも動かさない。）

@DEV: ...Although, if you've ever built hardware, you already know: the war over dimensions never really ends. Better close my CAD window before I get any ideas. See you on the next episode of Off the Breadboard.
# （……もっとも、ハードウェアを作ったことがあるなら、あんたももう知っているはずだ。寸法を決める戦いに、本当の終わりはない。変な気を起こす前にCADのウィンドウを閉じておくとしよう。また次回の『Off the Breadboard』で会おう。）
