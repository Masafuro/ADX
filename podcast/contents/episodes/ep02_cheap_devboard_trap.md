# Episode 02: Don't Build a Cheap Microcontroller Board (Hard Mode With Zero Budget)
# Off the Breadboard — Stage 2 Production Script (Converted from drafts/ep02_stage1_cheap_devboard_trap.md)

# [VISUAL: ADX CORE-U archive graveyard next to an industrial CORE-A board]
@DEV: If you're thinking about launching a cheap, general-purpose microcontroller board to enter the hardware market—honestly? Don't do it.
# （もし今、安くて汎用的なマイコンボードを作ってハードウェア市場に参入しようと考えているなら――正直に言う。やめておけ。）

@DEV: I'm serious. Building a cheap, general-purpose board is way harder than designing an expensive industrial controller.
# （本気で言っている。安くて汎用的なボードを作るほうが、高価な産業用コントローラを設計するよりはるかに難しいんだ。）

# [PAUSE 0.4s]
@DEV: Now, I know what you're thinking. "Wait—you design rugged industrial boards like ADX CORE A with isolated buses and strict factory specs. A budget USB board is just an eight-bit chip, a USB port, and a few LEDs. You can draw that before lunch!"
# （今あんたが思ってることは分かってる。「待てよ――お前は絶縁バスや厳しい工場仕様を備えたゴツい産業用ボードの ADX CORE A まで設計してるんだろ？ 格安USBボードなんて8ビットチップとUSBポートとLED数個じゃないか。昼飯前に描けるだろ！」ってな。）

# [VISUAL: Volume comparison — Millions/year (Raspberry Pi Pico & Arduino) vs. ~100/year (ADX CORE-U)]
@DEV: Drawing the schematic is easy. Surviving the math is a bloodbath.
# （回路図を描くのは簡単さ。数字の現実を生き残るのが血みどろなんだ。）

@DEV: Raspberry Pi ships over eight million microcontrollers and Pico boards every year. Arduino has passed forty million official boards—roughly a million UNOs a year—not even counting the millions of dirt-cheap clones flooding the market.
# （Raspberry Pi は毎年800万ユニット以上のマイコンや Pico を出荷している。Arduino も公式だけで累計4,000万台を突破し、UNO 単体でも年100万台ペース――しかも市場に溢れる何百万台もの激安クローンを数に入れる前の話だ。）

@DEV: And our indie budget board, ADX CORE U? Roughly one hundred boards a year.
# （そして俺たちのインディー格安ボード『ADX CORE U』は？ だいたい年間100台だ。）

# [PAUSE 0.4s]
@DEV: Do that math with me. Millions a year versus one hundred—a ten-thousand-to-one volume gap. At a hundred boards a year, stencil setup, reel changes, and small-batch assembly eat you alive. Fighting head-to-head on specs, price, and stock is bringing a pocketknife to an orbital laser strike. You cannot win—and that's why ADX CORE U got killed and archived over and over again.
# （一緒に計算してみよう。年間数百万台に対して年間100台――生産規模の差は1万倍以上だ。年100台ロットでは、ステンシル代、リール交換、小ロット実装費が容赦なく利益を食い尽くす。性能・価格・在庫で正面から殴り合うのは、軌道レーザー兵器にポケットナイフで挑むようなものだ。勝てるわけがない――だからこそ、ADX CORE U は何度も廃版になってアーカイブ送りになったんだよ。）

# [PAUSE 0.5s]
# [VISUAL: Electronics aisle (r/arduino, r/raspberrypi, r/embedded) vs. 3D printer shop shelf next to PLA filament (r/3Dprinting)]
@DEV: So why did we keep dragging it back out? Because we realized we were standing on the wrong shelf.
# （じゃあ、なぜ俺たちは何度も引っ張り出してきたのか？ 立つ棚を間違えていたと気づいたからだ。）

@DEV: Put a new microcontroller board in the electronics aisle—right next to an Arduino UNO and a four-dollar Pico, pitching to the Arduino, Raspberry Pi, and embedded subreddits—and you are dead on arrival.
# （新しいマイコンボードを電子工作の棚――Arduino UNO や4ドルの Pico のすぐ横に並べて、r/arduino や r/raspberrypi、r/embedded に売り込んだら、並んだ瞬間に即死する。）

@DEV: You have to leave the electronics store completely and place it in a 3D printer shop, right next to the spools of PLA filament—moving over to the 3D printing community. Who is actually printing boxes every day and fighting to put electronics inside them? Not spec-sheet warriors—makers building real, enclosed tools.
# （電子パーツ屋から完全に撤退して、3Dプリンター販売店のPLAフィラメントのすぐ横に置くんだ――Redditで言えば r/3Dprinting への出奔だ。毎日実際に箱を刷って、その中に電子回路を押し込むのに苦労してるのは誰だ？ スペック戦士じゃない――実用的な筐体入りの道具を作っているメイカーたちだ。）

# [PAUSE 0.4s]
# [VISUAL: CORE-U rev1 → rev4 subtraction — Full SMD, dropping buzzer, Grove headers, and physical RESET]
@DEV: Still, switching shelves doesn't erase the hundred-board math. At small-batch scale, one hand-soldered through-hole part or one extra gimmick wrecks your unit cost.
# （とはいえ、棚を変えても「年100台」の数字の呪縛は消えない。小ロット規模では、手はんだのスルーホール部品が1個、おまけ機能が1個増えるだけで単価が崩壊する。）

@DEV: So we stripped ADX CORE U naked. Full SMD. No piezo buzzer. No Grove headers. No physical reset button—because inside a screwed-shut 3D-printed box, a tactile button is useless anyway. Strip the gimmicks, drop the cost, and guarantee rock-solid flashing and debugging with the lid closed.
# （だから俺たちは ADX CORE U を丸裸に削ぎ落とした。全部品フルSMD。圧電ブザーなし。Groveコネクタなし。物理リセットボタンもなし――ネジで閉じた3Dプリント筐体の中じゃ、タクトスイッチなんてどうせ押せないからな。おまけを全部捨ててコストを落とし、蓋を閉めたまま確実に書き込めてデバッグできることだけに特化した。）

# [PAUSE 0.5s]
# [VISUAL: Smartphone USB-C ecosystem chaos — 500mA laptop ports vs. 5V/2A bricks vs. back-powering]
@DEV: Clean, right? Until you hit the final boss of budget hardware: USB Type-C. One port, or two?
# （綺麗だろ？ ――格安ハードウェアのラスボスにぶつかるまではな。USB Type-C だ。1ポートか、2ポートか。）

@DEV: Blame smartphones. Smartphones put USB Type-C into every drawer on Earth—and dragged a nightmare of power chaos right along with it: weak five-hundred-milliamp laptop ports, five-volt two-amp wall bricks, back-feeding rails. And no matter how clever your schematic is, one tiny budget board cannot fix USB Type-C's chaos. You have to give something up.
# （スマホのせいだ。スマホが地球上のあらゆる引き出しに USB Type-C を普及させた――そして同時に電力供給のカオスという悪夢も持ち込んだ。500mAしか出ないPCポート、5V/2AのACアダプタ、電源の逆流。どれだけ回路図を工夫しても、小さな格安ボード1枚で USB Type-C 規格の混乱は解決できない。何かを諦めるしかないんだ。）

# [PAUSE 0.4s]
# [VISUAL: Three impossible doors — Door 1: Cost (ORing IC) / Door 2: 10W Power (1 Port) / Door 3: 1-Cable Convenience (2 Dedicated Ports)]
@DEV: There are only three doors—and every one bites you.
# （ドアは3つしかない――そしてどれを開けても噛みつかれる。）

@DEV: Door one: give up cost. Add an ORing power-controller chip so either port works seamlessly like an Arduino. Except at a hundred boards a year, one power-mux IC blows up your BOM—and two USB-C ports still invite miswiring.
# （ドア1：コストを諦める。ORing コントローラICを積んで、どちらのポートからでも Arduino のように使えるようにする。だが年100台ロットでは、パワーマルチプレクサICを1個足すだけでBOMが吹っ飛ぶ――しかも Type-C が2口ある限り、誤配線は消えない。）

@DEV: Door two: give up power. Drop five volts at two amps—ten watts—and stick to one clean USB-C port. Cheap, zero confusion—until someone drives LEDs or a motor inside their box from a laptop port and everything browns out.
# （ドア2：パワーを諦める。5V/2A・10W を捨てて、USB-C を1本に絞る。安くて混乱もゼロ――ただし箱の中のLEDやモーターをノートPCから動かそうとした瞬間、電圧降下で落ちる。）

@DEV: Door three: give up the one-cable Arduino magic. Split power and data into two dedicated USB-C ports. No ORing chip, full ten-watt power intact, stays affordable.
# （ドア3：ケーブル1本の Arduino 的手軽さを諦める。電源と通信を2つの専用 USB-C ポートに分離する。ORing ICは不要、10Wパワーも維持できて、安く作れる。）

# [PAUSE 0.4s]
@DEV: On ADX CORE U, we picked door three. And I know what you're yelling right now: "If both ports are USB Type-C, someone is STILL going to plug a charger into the data port and think the board is dead!" You're right. Even door three doesn't magically erase USB-C miswiring. There's no textbook win here—just picking which poison won't kill the project.
# （ADX CORE U で選んだのはドア3だ。そして今、あんたが叫んでることも分かってる。「両方 USB Type-C なら、絶対にデータポートに充電器を挿して『壊れてる！』ってなるやつが出るぞ！」――その通りだ。ドア3を選んでも、USB-C の誤配線は魔法のように消えない。ここには教科書の勝利なんてない。プロジェクトを殺さない毒を選ぶだけなんだ。）

# [PAUSE 0.6s]
# [VISUAL: Industrial CORE-A vs. Budget CORE-U — "Hard mode with zero budget"]
@DEV: And that's the irony. On high-value industrial boards like ADX CORE A, you don't fight this war. At a hundred units a year, an industrial price tag absorbs small-batch costs, proper power rails, and dedicated terminals—with zero consumer USB-C trap.
# （そしてそこに皮肉がある。産業用の ADX CORE A みたいな高付加価値ボードなら、こんな戦いは起きない。年100台ロットでも、産業用の売価が小ロット製造費もちゃんとした電源回路も専用端子も吸収してくれる――民生 USB-C の罠とも無縁だ。）

@DEV: People see a cheap, general-purpose board and assume it's an easy place to start. Total opposite. Cheap hardware isn't easy mode—it's hard mode with zero budget.
# （みんな安くて汎用的なボードを見ると、簡単そうな入門編だと思い込む。真逆だ。安いハードウェアはイージーモードじゃない――予算ゼロのハードモードなんだ。）

# [PAUSE 0.5s]
@DEV: So if you're tempted to launch a cheap microcontroller board from scratch? Take it from someone who has killed ADX CORE U more times than I can count: honestly, don't do it.
# （だから、一から安価なマイコンボードを出してみたいという誘惑に駆られているなら？ ADX CORE U を数えきれないほど廃版にしてきた人間からのアドバイスだ――ぶっちゃけ、やめた方がいい。）

@DEV: ...Anyway, I'm pushing the next revision tonight. See you on the next episode of Off the Breadboard.
# （……ま、それはそれとして、俺は今夜も次のリビジョンをプッシュするんだけどな。また次回の『Off the Breadboard』で会おう。）
