# Stage 1 Dialectic Draft (v1 Engine): Episode 04
# Theme: なぜマイコン基板の「USB Type-C 2本並び」はダサいのか？ — Type-C帝国の呪いと「2セント差」の別基板税
# Framework: talk_show_bible.md (4 Cardinal Rules / 4-Act Review Protocol / Mode A: Pre-Flight Review)
# Source Memo: podcast/contents/memo/0004_なぜマイコン基板のUSB-C2本並びはダサいのか.md / memo/SMOKINGROOM/2026-11-23_8858_chr4_and_the_curse_of_usb_c.md

# --- Phase 1: The Agenda & The Taboo ---

@ECHO: Dev. I am looking at the repository commit log, and I believe I just witnessed a new land-speed record in bad project management. You designed a brand-new sixteen-component external programmer probe, generated a full netlist, priced a hundred and sixty units at the factory... and threw it straight into the `archive/` graveyard on the exact same day.
# （Dev。リポジトリのコミットログを見ていますが、プロジェクト管理の迷走スピード世界新記録を目撃した気がします。16部品の完全新規な外付け書き込みプローブを設計し、フルネットリストを出し、工場で160個の見積もりを取り……生まれたその日のうちに `archive/` 墓場へ直行させましたよね。）

@DEV: Born at noon, buried before dinner. Clean funeral.
# （昼に生まれて、晩飯前に埋葬した。綺麗な葬式だろ。）

@ECHO: Why did you even build it in the first place?! Just two episodes ago, we locked in dual USB Type-C ports on ADX CORE-U—one for power, one for data. Zero back-feeding, no expensive power-mux chip, full ten-watt power! Electrically, it passed every single check. Why on earth did you suddenly try to rip the programmer off the board?!
# （そもそもなぜ最初からそれを作ったんですか？！ たった2話前に、ADX CORE-U は電源用とデータ用のデュアル USB Type-C で確定したじゃないですか。逆流ゼロ、高価なパワーマルチプレクサ不要、10W 給電も満額クリア！ 電気的には全項目合格でしたよ。なぜ急に基板から書き込み機能を引っこ抜こうとしたんですか？！）

@DEV: Because two USB-C ports sitting side by side on a microcontroller board look terribly ugly.
# （マイコン基板の上に USB-C ポートが2個並んでいるのが、どうしようもなくダサく見えたからだ。）

# --- Phase 2: The Sarcastic Invalidation ---

@ECHO: ...Excuse me? Did my audio decoder glitch, or did a grown hardware engineer just burn an entire afternoon designing a sixteen-part PCB because of *aesthetic vibes*?!
# （……失礼ですが？ 私の音声デコーダがバグったのでしょうか、それともいい大人のハードウェア設計者が、ただの「見た目のノリ」のために丸半日かけて16部品の基板を設計したのですか？！）

@ECHO: And what is even "ugly" about two USB ports next to each other?! Look at your laptop. Look at the back of your desktop PC—four, six, eight USB Type-A ports lined up in a crisp metallic row! Nobody looks at a computer with multiple USB ports and bursts into tears!
# （それに、USBポートが2つ並んでいることの何が「ダサい」んですか？！ あなたのノートPCを見てください。デスクトップPCの背面を見てください――4個も6個も8個も、USB Type-A ポートが金属の列を作って並んでいますよ！ 複数のUSBポートを見て泣き崩れる人間なんていません！）

@DEV: That's completely different. Four USB-A ports on a PC are symmetrical slots—they're a badge of capability. Plug a keyboard, a mouse, or a drive into any hole you want, and it works.
# （それは全くの別物だ。PC に並ぶ4つの USB-A ポートは対等なスロット――「能力の象徴」だよ。キーボードでもマウスでもドライブでも、どの穴に挿しても動く。）

@DEV: Now look at those modern dual-USB-C ESP32 dev boards—or our own board. Two identical silver twins staring right at you. Except one is secretly "UART" and the other is "USB." Or one is "power only" and the other is "data only." Every single time you pick it up, you have to tilt the board under a desk lamp and squint at microscopic silkscreen just to remember which twin does what.
# （今度は最近のデュアル USB-C 仕様の ESP32 開発ボード――あるいは俺たちのボードを見てみろ。まったく同じ銀色の双子がこっちを睨んでいる。なのに片方はこっそり「UART」で、もう片方は「USB」。あるいは片方は「電源専用」で、もう片方は「通信専用」だ。手に取るたび、基板をデスクライトの下で傾けて米粒みたいなシルク印刷を覗き込まなきゃ、どっちの双子が何担当か思い出せない。）

@ECHO: Hold on... so you're saying two identical USB-C ports on a microcontroller aren't a feature—they're an exposed confession?! Like taping a sticky note to the front of your hardware that says, *"I really wanted one smart cable, but my chip and BOM couldn't pull it off, so here are two identical holes—good luck!"*?!
# （待ってください……つまりマイコン基板の同じ形の USB-C 2本並びは「機能」ではなく「設計の言い訳の露出」だと？！ ハードウェアの顔面に「本当はスマートな1本にしたかったんですが、チップとBOMの都合でまとめられなかったので同じ穴を2個開けました、頑張って当ててください！」と付箋を貼っているようなものだと？！）

@DEV: Exactly. USB Type-C made the world one promise: *one universal cable for power and data.* Put two identical USB-C jacks side by side where one refuses data and the other refuses power, and your board breaks that promise on sight. Your gut knows it's a compromise in half a second.
# （その通り。USB Type-C は世界にたった1つの約束をした――「電源もデータも万能の1本で完結する」と。それなのに、片方はデータを拒み、もう片方は電源を拒む同じ形のジャックを2つ並べた瞬間、見ただけでその約束を裏切る。人間の直感は0.5秒で「あ、妥協だな」と見抜くんだよ。）

@ECHO: Fine! I admit, you just ruined every dual-USB-C ESP32 board on my shelf. Then WHY did you bury your external programmer probe?! If splitting the programmer off cleans up the board to one port and drops the board cost, why didn't you ship it?!
# （結構です！ 認めましょう、今あなたのせいで私の棚にあるデュアル USB-C の ESP32 ボードが全部ダサく見え始めました。なら、なぜ外付けプローブを埋葬したんですか？！ 書き込み機能を切り離せば基板はスッキリ1ポートになって安くなるはずでしょう、なぜ出荷しないんですか？！）

# --- Phase 3: The Reality-Check & Math ---

@DEV: Because the factory quote hit me with a two-cent brick wall. Look at the actual numbers, assembled and shipped from the fab.
# （工場の見積もりが「2セントのレンガ壁」を顔面に叩きつけてきたからだ。実装・送料込みの実際の数字を見ろ。）

@DEV: Our full `CORE-U` board—microcontroller, dual USB-UART bridge, eFuse, LEDs, twenty-pin bus, and both ugly USB-C ports—costs three dollars and eighty-three cents per unit. At a standard four-times retail rule, that's about fifteen dollars and thirty-five cents.
# （全部入りの `CORE-U` ボード――マイコン、デュアルUSB-UARTブリッジ、eFuse、LED、20ピンバス、そしてダサい2個のUSB-Cポート込みで――1個あたり3ドル83セント。標準の4倍算で売価は約15ドル35セントだ。）

@DEV: Now look at the bare-bones, sixteen-part external probe I just designed to escape that second port: three dollars and eighty-one cents per unit. Retail: fifteen dollars and thirty cents.
# （今度は、2本目のポートから逃げるために俺がさっき設計した最小構成16部品の外付けプローブを見ろ。1個あたり3ドル81セント。想定売価は15ドル30セントだ。）

@ECHO: ...Wait. Let me double-check my floating-point unit. You stripped the board down to a tiny sixteen-component programmer dongle... and it only came out TWO CENTS cheaper than the entire microcontroller board?! How is that mathematically possible?!
# （……待ってください、浮動小数点ユニットを再計算します。たった16部品の極小書き込みドングルまで削ぎ落としたのに……フル装備のマイコンボードより「たった2セント」しか安くならなかったと？！ 数学的にどうやったらそうなるんですか？！）

@DEV: Call it the Separate-SKU Tax. When a programmer rides along on your main board, you only pay for the extra silicon—marginal cost. The moment you spin it out as a separate product, you pay all the fixed costs a second time: another bare PCB, another SMT setup, another shipment, another ESD bag, another SKU. You can delete half the parts and you will NEVER get a small-batch indie dongle down to five dollars. Five dollars barely pays for an unpopulated board!
# （「別SKUの独立税」と呼べ。書き込み機能がメイン基板に相乗りしているうちは、追加シリコンの限界コストしか払わなくていい。だが別製品として切り出した瞬間、あらゆる固定費をもう一回丸ごと払うことになる――別の生基板、別のSMT段取り、別の送料、別の静電気袋、別の在庫管理。部品を半分削っても、小ロットのインディー製品で5ドルドングルなんて絶対に作れない。5ドルは実装なしの生基板レベルだ。）

@DEV: And then you fall into the Valley of Doom. A fifteen-dollar probe plus a fifteen-dollar stripped board equals thirty dollars—dead against a twenty-dollar Arduino UNO R4 Minima. Meanwhile, our full RP2040 industrial probe costs only two dollars and twenty-one cents more to build, with PIO and two megabytes of flash. A cheap cut-down probe dies from below AND dies from above.
# （しかもそこから「魔の谷」へ転落する。15ドルのプローブ＋15ドルの廉価基板＝合計30ドル――20ドルの Arduino UNO R4 Minima に正面から潰される。一方で、うちの本格派 RP2040 プローブなら製造原価はたった2ドル21セント高いだけで、PIOも2MBフラッシュも積んでいる。中途半端な格安プローブは下からも上からも挟まれて即死するんだ。）

@ECHO: Alright, fine—splitting the programmer off is economic suicide. Then solve the ugliness the way hardware engineers solved it for forty years! Use two DIFFERENT connectors! Put a classic round DC barrel jack for power and a chunky USB Type-B for data, just like an Arduino UNO! Your fingers know which hole is which in a tenth of a second without reading a single millimeter of silkscreen!
# （分かりました、結構です――書き込み機能の外出しは経済的自殺ですね。なら、ハードウェア技術者が40年間やってきた王道でダサさを解決すればいいじゃないですか！ 「異種2ポート」にしてください！ Arduino UNO みたいに、電源は丸いDCバレルジャック、データはゴツい USB Type-B！ シルク印刷を1ミリも読まなくても、指先が0.1秒でどっちの穴か判別できますよ！）

@DEV: That's the textbook answer—and it's dead. Because USB Type-C killed it.
# （それが教科書の正解だ――そして、もう死んでいる。USB Type-C が殺したからだ。）

@DEV: Think about living in 2026. If you ship a maker board today that demands a five-volt DC barrel adapter or a square USB Type-B printer cable, how many people even own one? Unless you keep a dusty inkjet printer in the closet, USB Type-B is a museum exhibit. Even five-volt barrel bricks are turning into specialty hobby parts.
# （2026年の現実を考えてみろ。今日び、5VのDCバレルアダプタや四角いUSB Type-Bプリンタケーブルを要求するボードを出荷して、何人がそれを持っている？ 押入れに埃をかぶったインクジェットプリンタでもない限り、USB Type-B は博物館の展示物だ。5VバレルACアダプタですら、いまやマニア向けの専門部品になりつつある。）

@DEV: Meanwhile, USB Type-C conquered the planet. If your cable snaps at midnight, you can walk into a convenience store or a dollar shop and buy a USB-C cable for two bucks—well, assuming you don't accidentally grab a sneaky charge-only cable with no data wires inside.
# （その一方で、USB Type-C は地球を征服した。真夜中にケーブルが断線しても、コンビニや100円ショップに歩いて行けば数ドルで USB-C ケーブルが手に入る――もっとも、中にデータ線の入っていない罠の「充電専用ケーブル」を掴まされなければの話だがな。）

@ECHO: *(Sighs deeply)* ...Hold on. Let me trace this tragedy. Availability forces every modern board to use USB Type-C... while USB Type-C wiped out the physical variety of connectors that gave hardware clear affordance in the first place?! So the entire hardware industry is basically being held hostage by the USB-C Empire—and every awkward dual-USB-C ESP32 board on the market is a cultural fossil of a world where one plug ate all the other shapes?!
# （（深い溜息）……待ってください。この悲劇を整理させてください。入手性のせいで現代の全ボードが USB Type-C を使わざるを得ないのに……その USB Type-C 自身が、ハードウェアに分かりやすいアフォーダンスを与えていた「異種コネクタ文化」を絶滅させてしまったと？！ つまりハードウェア業界全体が「USB-C帝国」の人質に取られており、市場のぎこちないデュアル USB-C の ESP32 ボードは全部、1つのプラグが他の全ての形を食い尽くした世界の民俗資料・化石だと？！）

# --- Phase 4: The Bitter Sign-off ---

@DEV: Pretty much. We killed the external probe, kept the two silver twins, and accepted our fate as citizens of the USB-C Empire.
# （だいたいそんなところだ。外付けプローブは埋葬し、銀色の双子を残し、USB-C帝国の市民としての運命を受け入れたよ。）

@ECHO: *(Pained silence)* ...I hate that your economics AND your industrial design critique both check out. Archive commit for the probe: APPROVED. Though I must say—one universal connector to rule them all, and yet here we are squinting at tiny silkscreen labels between two identical sockets, praying a corner-store cable actually has data pins.
# （（苦渋の沈黙）……あなたの経済計算も、インダストリアルデザイン論も、両方とも筋が通っているのが本当に腹立たしいですね。プローブのアーカイブ・コミット：APPROVED（承認）。ただ一言だけ言わせてください――すべてを統べる万能の1本のはずだったのに、俺たちは同じ形の2つの穴の間で小さなシルクを睨みつけ、コンビニのケーブルにデータ線が入っていることを祈っているんですね。）

@DEV: Makes you laugh, doesn't it? Are we sure humanity is actually moving forward?
# （笑えてくるだろ？ 人類って、本当に進歩してるのかね？）

---

## Stage 2 へ引き継ぐ核（4要素メモ）

1. **The Taboo（DEVの禁忌 / やらかし自白）**:
   * 「16部品の完全新規な外付けプローブを昼に設計して工場見積もりまで出し、**生まれたその日の晩飯前にアーカイブ墓場へ埋葬**した。きっかけは電気的不具合じゃない――『マイコン基板の USB-C 2本並びが、どうしようもなくダサい』という純粋な美意識だった。」
2. **The Sarcastic Attacks（ECHOの猛攻 → 聴き手 `"You"` の疑問・反論へ変換）**:
   * 「見た目のノリだけで丸半日かけた基板をボツにしたのか？ そもそもPCの背面にはUSBポートが4つも6つも並んでいるのに、なんでマイコンのUSB-C 2本並びだけダサいんだよ？」
   * 「そんなに2ポートが嫌なら、書き込み機能を外付けの安いドングルに切り離せばいいじゃないか！」
   * 「じゃあ昔ながらの Arduino UNO みたいに、DCバレルジャック＋USB Type-B の異種2ポートにすれば一発で解決だろ！」
3. **The Metaphor Bite（比喩翻訳ツッコミ — Stage 2 でDEVの決め台詞として残す核）**:
   * *Four USB-A ports on a PC are symmetrical slots—a badge of capability. Two identical USB-C ports on a microcontroller board are exposed design excuses.*（PCのUSB-A並列は「能力の象徴」、マイコン基板のUSB-C双子は「まとめられなかった設計の言い訳の露出」）
   * *Two identical silver twins wearing secret jobs, forcing you to tilt the board under a lamp and squint at microscopic silkscreen.*（秘密の役割を背負った同じ銀色の双子。毎回ランプの下で傾けて極小シルクを覗き込む羽目になる）
   * *The Separate-SKU Tax*（別SKUの独立税：相乗りなら限界コストだが、切り出した瞬間に生基板・SMT・送料・袋の固定費が丸ごと二重に乗る）
   * *The USB-C Empire*（USB-C帝国：入手性のインフラになった代わりに、指先で0.1秒で判別できた異種コネクタ文化を絶滅させた。Type-Bや5Vバレルジャックは博物館の展示物）
   * *2020s engineering fossils of a world where one plug ate every other shape*（1つのプラグが他のすべての形を食い尽くした2020年代の工学化石）
4. **The Math / Reality Payoff（反論不能な数字・普遍的哲学）**:
   * **「2セント差」のレンガ壁**：全部入りの `ADX CORE U`（`$3.83` / 4倍算 `$15.35`） vs 最小16部品の外付けプローブ `ADX CHR 4`（`$3.81` / 4倍算 `$15.30`）＝ **差額たった2セント**。小ロットで `$5` ドングルは物理的に不可能。
   * **「魔の谷」**：`$15` プローブ＋`$15` ボード＝`$30`（`$20` の Arduino UNO R4 Minima に下から潰され、原価差たった `$2.21` のフルスペック RP2040 プローブ `ADX RPR 4` に上から潰される）。
   * **結びの皮肉（脱・説教）**：「すべてを統一する万能の1本のはずだった未来で、俺たちは同じ形の穴を2つ並べて小さなシルクを覗き込み、コンビニで買ったケーブルにデータ線が入っていることを祈っている。……俺たちは本当に進歩しているんだろうか？」
