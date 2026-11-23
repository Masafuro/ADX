# Episode 04: Why Dual USB-C Ports Look Ugly on ESP32 & MCU Boards—And the 2-Cent Trap
# Off the Breadboard — Stage 2 Production Script (Converted from drafts/ep04_stage1_dual_usbc_curse.md)

# [VISUAL: Git commit log showing CHR4_Smart_Probe born and archived on the same day]
@DEV: Today, I set a personal speedrun record: I designed a sixteen-component external programmer at noon, priced a hundred and sixty units at the factory, and buried it in the archive graveyard before dinner.
# （今日、俺は個人スピードラン記録を更新した。昼に16部品の外付け書き込みプローブを設計し、工場で160個の見積もりを取り、晩飯前にはアーカイブの墓場へ埋葬したんだ。）

@DEV: And I didn't try to rip the programmer off ADX CORE U over a circuit bug. I did it for one embarrassingly honest reason: two USB Type-C ports sitting side by side on a microcontroller board look terribly ugly.
# （しかも ADX CORE U から書き込み機能を引っこ抜こうとした理由は回路バグじゃない。たった1つの馬鹿正直な理由だ――マイコン基板の上に USB Type-C ポートが2個並んでいるのが、どうしようもなくダサく見えたんだよ。）

# [PAUSE 0.4s]
@DEV: Now I know what you're thinking. "Wait—two episodes ago you proved dual USB-C works! Zero back-feeding, no expensive power-mux chip, full ten-watt power! And what's ugly about multiple USB ports?! Desktop PCs have four or six USB-A ports lined up in a row, and nobody cries about that!"
# （今あんたが思ってることは分かってる。「おい待て――2話前にお前自身、デュアル USB-C は電気的に成立するって証明しただろ！ 逆流ゼロ、高価なパワーマルチプレクサ不要、10W給電もクリア！ そもそもUSBポートが並ぶことの何がダサいんだ？！ デスクトップPCには USB-A が4個も6個も並んでるが、誰も泣かないぞ！」ってな。）

# [PAUSE 0.4s]
# [VISUAL: Desktop PC symmetrical USB-A row vs. dual USB-C ESP32 / MCU board with tiny "PWR / COM" or "UART / USB" silkscreen]
@DEV: Think about the difference. Four USB-A ports on a PC are symmetrical slots—a badge of capability. Plug a keyboard, mouse, or drive into any hole, and it just works.
# （違いを考えてみろ。PCに並ぶ4つの USB-A ポートは対等なスロット――「能力の象徴」だ。キーボードでもマウスでもドライブでも、どの穴に挿してもそのまま動く。）

@DEV: Now pick up a modern dual-USB-C ESP32 dev board—or our own ADX CORE U. Two identical silver twins staring right at you. Except one is secretly "UART" and the other is "USB." Or one takes power and the other takes data. Every time you grab a cable, you tilt the board under a lamp and squint at microscopic silkscreen to remember which twin does what.
# （今度は最近のデュアル USB-C の ESP32 開発ボード――あるいは俺たちの ADX CORE U を手に取ってみろ。まったく同じ銀色の双子がこっちを睨んでいる。なのに片方はこっそり「UART」、もう片方は「USB」。あるいは片方は電源、もう片方はデータだ。ケーブルを挿すたび、基板をライトの下で傾けて米粒みたいなシルク印刷を覗き込まなきゃ、どっちの双子が何担当か思い出せない。）

@DEV: USB Type-C made the world one promise: one universal cable for power and data. Put two identical USB-C jacks side by side where one refuses data and the other refuses power, and you break that promise on sight. Those two sockets aren't a feature—they're an exposed design excuse, taped to the front of your board. Your gut catches the compromise in half a second.
# （USB Type-C は世界に1つだけ約束をした――「電源もデータも万能の1本で完結する」と。なのに、片方はデータを拒み、もう片方は電源を拒む同じ形のジャックを2つ並べた瞬間、見ただけでその約束を裏切る。あの2つの穴は機能じゃない――基板の顔面に貼り付いた「設計の言い訳」の露出だ。人間の直感は0.5秒で妥協を見抜く。）

# [PAUSE 0.5s]
# [VISUAL: Factory quote comparison — ADX CORE-U ($3.83) vs. 16-part CHR4 external probe ($3.81) = 2-cent gap]
@DEV: So you're asking, "Then why did you bury your external programmer? Split it off, keep one clean port, and sell a five-dollar dongle!"
# （なら、あんたはこう訊くはずだ。「だったらなんで外付けプローブを埋葬したんだ？ 切り離してスッキリ1ポートにして、安い5ドルドングルで売ればいいじゃないか！」）

@DEV: I tried. I built a bare-bones, sixteen-part external probe—ADX CHR 4—and ran an assembled, shipped factory quote for a hundred and sixty units. And the math hit me with a two-cent brick wall.
# （やってみたさ。最小構成16部品の外付けプローブ――ADX CHR 4――を組んで、160個ロットの実装・送料込みの見積もりを工場で弾いた。そしたら数字が「2セントのレンガ壁」を叩きつけてきたんだ。）

@DEV: Our full ADX CORE U board—microcontroller, dual USB-UART bridge, eFuse, LEDs, twenty-pin bus, and both ugly USB-C ports—costs three dollars and eighty-three cents to build. At a normal four-times retail rule, that's about fifteen dollars and thirty-five cents.
# （全部入りの ADX CORE U ボード――マイコン、デュアルUSB-UARTブリッジ、eFuse、LED、20ピンバス、例のダサい2個のUSB-Cポート込み――の製造原価は3ドル83セント。標準の4倍算で売価は約15ドル35セントだ。）

@DEV: That stripped-down, sixteen-part external probe? Three dollars and eighty-one cents. Retail: fifteen dollars and thirty cents.
# （じゃあ、あの削ぎ落とした16部品の外付けプローブはいくらか？ 3ドル81セント。想定売価は15ドル30セントだ。）

@DEV: Two cents. A full microcontroller board and a tiny programmer dongle landed two cents apart.
# （たった2セントだ。フル装備のマイコンボードと極小ドングルが、わずか2セント差に並んだんだよ。）

# [PAUSE 0.4s]
@DEV: Welcome to the Separate-SKU Tax. On your main board, a programmer rides on marginal silicon cost. Spin it onto its own PCB, and you pay every fixed cost twice—another bare board, SMT setup, shipping, and ESD bag. Delete half the parts and you will still never hit a five-dollar indie dongle; five dollars barely buys an unpopulated board.
# （ようこそ、「別SKUの独立税」の世界へ。メイン基板に相乗りしていればシリコンの限界コストだけで済む。だが別基板に切り出した瞬間、別の生基板、SMT段取り、送料、静電気袋という固定費を丸ごと二度払うことになる。部品を半分削ってもインディー規模で5ドルドングルは絶対に作れない――5ドルじゃ実装なしの生基板が精一杯だ。）

@DEV: Pair a fifteen-dollar probe with a fifteen-dollar board and you hit thirty dollars—crushed from below by a twenty-dollar Arduino UNO R4 Minima, and from above by our full RP2040 probe, ADX RPR 4, which costs just two dollars and twenty-one cents more to build with PIO and two megabytes of flash. No wonder ADX CHR 4 died before dinner.
# （15ドルのプローブと15ドルの基板を合わせれば30ドル――下からは20ドルの Arduino UNO R4 Minima に潰され、上からは原価差たった2ドル21セントでPIOも2MBフラッシュも積む俺たちの本格派 RP2040 プローブ、ADX RPR 4 に潰される。ADX CHR 4 が晩飯前に死んだのも当然だ。）

# [PAUSE 0.5s]
# [VISUAL: Classic Arduino UNO (round DC barrel jack + square USB-B) fading into museum glass, replaced by a convenience-store USB-C wall]
@DEV: I can hear your last comeback already. "Fine—keep two ports, just make them DIFFERENT connectors! A round DC barrel jack for power and a chunky USB Type-B for data, like an Arduino UNO! Your fingers know which hole is which in a tenth of a second!"
# （あんたの最後の切り返しも聞こえてるぞ。「分かった、2ポートのままでいい――ただし『異種コネクタ』にしろ！ Arduino UNO みたいに、丸いDCバレルジャックとゴツい USB Type-B だ！ 指先が0.1秒でどっちの穴か分かるだろ！」ってな。）

@DEV: That's the textbook answer—and USB Type-C killed it.
# （それが教科書の正解だ――そして USB Type-C がそいつを殺した。）

@DEV: Try shipping a maker board today that demands a square USB Type-B cable or a five-volt barrel brick. Unless you keep a dusty printer in the closet, USB Type-B is a museum exhibit, and five-volt barrel adapters are turning into specialty hobby parts.
# （今日び、四角い USB Type-B ケーブルや5VのバレルACアダプタを要求するメイカー向けボードを出してみろ。押入れに埃をかぶったプリンタでもない限り USB Type-B は博物館の展示物だし、5Vバレルアダプタもマニア向けの専門部品になりつつある。）

@DEV: Meanwhile, USB Type-C conquered the planet. Snap a cable at two in the morning, and you can grab a replacement at any convenience store or dollar shop—assuming you don't accidentally buy a sneaky charge-only cable with zero data wires inside.
# （その一方で、USB Type-C は地球を征服した。深夜2時にケーブルが断線しても、コンビニや100円ショップで代わりが手に入る――もっとも、中にデータ線の入っていない罠の「充電専用ケーブル」を掴まされなければの話だがな。）

# [PAUSE 0.5s]
@DEV: That's the trap. Availability forces every board onto USB Type-C—while USB Type-C wiped out the physical variety of plugs that made hardware effortless to read by touch. Those awkward dual-USB-C ESP32 boards aren't laziness; they're 2020s engineering fossils—proof we're all hostages of the USB-C Empire.
# （それが罠なんだ。入手性が全ボードに USB Type-C を強制する――その一方で、USB Type-C 自身が、触るだけで使い方が分かった異種プラグ文化を絶滅させてしまった。あのぎこちないデュアル USB-C の ESP32 ボードは怠慢じゃない。2020年代の工学化石だ――俺たち全員が「USB-C帝国」の人質だという証拠だよ。）

# [PAUSE 0.6s]
@DEV: So we buried the two-cent probe, kept the two silver twins on ADX CORE U, and surrendered to the empire.
# （というわけで、2セント差のプローブは埋葬し、ADX CORE U の銀色の双子を残して、帝国に降伏した。）

@DEV: Still, look at our shiny modern desks: one universal plug to rule them all—yet here we are, squinting at tiny silkscreen between two identical holes, praying a corner-store cable actually has data pins. Makes you laugh, doesn't it? Are we sure humanity is actually moving forward? See you on the next episode of Off the Breadboard.
# （それでも、ピカピカな現代の机を眺めてみろよ。すべてを統一する万能プラグのはずだったのに――俺たちは同じ形の2つの穴の間で小さなシルクを睨みつけ、コンビニのケーブルにデータ線が入っていることを祈っている。笑えてくるだろ？ 人類って、本当に進歩してるのかね？ また次回の『Off the Breadboard』で会おう。）
