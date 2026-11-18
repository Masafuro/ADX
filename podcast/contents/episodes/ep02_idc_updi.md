# Episode 02: Why I Put 12V High-Voltage UPDI Into a 20-Pin Ribbon Cable
# v5 Production Script — Converted from Stage 1 (v1/script_idc_updi.md) via 5 Conversion Rules

# [VISUAL: 20-pin IDC pinout diagram with Pin 14 highlighted "UPDI (12V HV CAPABLE)"]
@DEV: I just put UPDI on Pin 14 of our unshielded 20-pin ribbon cable. And if you know Microchip's UPDI recovery spec, you already know why that sounds insane.
# （シールド無しの20ピン・フラットケーブルの14番ピンにUPDIを載せた。MicrochipのUPDI復旧仕様を知っている人なら、これがどれほど狂った判断に聞こえるかもう分かるはずだ。）

# [PAUSE 0.4s]
@DEV: UPDI recovery uses a twelve-volt pulse. Right next to fragile 3.3-volt logic lines, inside a flat ribbon cable with zero shielding.
# （UPDI復旧には12ボルトのパルスを使う。シールドゼロのフラットケーブルの中で、繊細な3.3Vロジックの真横に、だ。）

@DEV: Now, I know what you're screaming at your headphones right now. "Are you trying to start an electrical fire? That is literally a miniature lightning bolt sitting millimeters away from your microcontroller!"
# （今あんたがヘッドホンに向かって叫んでる言葉は分かってる。「電気火災を起こす気か？ それ、マイコンの数ミリ横を走るミニチュアの雷そのものだぞ！」ってな。）

@DEV: And honestly? For three months, I agreed with you. I swore high voltage would never touch the IDC bus. If someone triggers a 12-volt pulse while a standard card is plugged in, you let the magic smoke out of the entire bus.
# （正直、3ヶ月間は俺もあんたに同意していた。高電圧だけは絶対にIDCバスに通さないと誓っていたんだ。標準カードが刺さった状態で誰かが誤って12Vパルスを撃ったら、バス全体から魔法の煙が吹き出して終わる。）

# [PAUSE 0.6s]
@DEV: So what changed my mind? Not a textbook. The factory floor.
# （じゃあ、なぜ方針をひっくり返したのか？ 教科書じゃない。工場の現場だ。）

# [VISUAL: 3mm VQFN-20 ATtiny1616 on oxidized OSP pad & bent pogo-pin jig]
@DEV: Picture what happens when you try to program a three-millimeter VQFN ATtiny1616 on an OSP-finished PCB. On paper, you just drop the board onto a bed-of-nails pogo-pin fixture and flash it.
# （OSP表面処理の基板に載った3ミリ角VQFNのATtiny1616を書き込む現場を想像してみてほしい。机の上では、ポゴピンの針山治具に基板を乗せて書き込むだけの簡単な話だ。）

@DEV: In reality, a bed-of-nails fixture is a medieval torture device. OSP pads oxidize. Contact resistance spikes. Pins bend, alignment drifts, and our production logs showed a fourteen percent false verification failure rate—meaning manual rework on boards that weren't even broken.
# （だが現実には、針山治具なんて中世の拷問器具だ。OSPパッドは酸化する。接触抵抗は跳ね上がる。ピンは曲がり、位置はズレる。結果、生産ログには14%もの書き込み偽エラーが並んだ。壊れてもいない基板を手作業でリワークする羽目になったんだ。）

@DEV: Now contrast that with putting UPDI on the IDC cable. You plug in ONE ribbon cable. Flash the bootloader, run the loopback tests, verify the transceiver, and lock the chip. Zero pogo pins. One click. Done.
# （一方で、IDCケーブルにUPDIを載せたらどうなるか。リボンケーブルを1本カチッと挿すだけだ。ブートローダーを書き込み、ループバック検査を回し、トランシーバを検証し、チップをロックする。針山ゼロ。ワンクリックで終わりだ。）

# [PAUSE 0.6s]
@DEV: Okay, great for yield—but you're still asking the obvious question: how do you stop that 12-volt pulse from incinerating every 3.3-volt chip on the bus?
# （よし、歩留まりには最高だ――だが、あんたはまだ当然の疑問を抱えているはずだ。「その12Vパルスがバス上の3.3Vチップを全部焼き尽くすのをどうやって防ぐんだ？」と。）

# [VISUAL: KiCad schematic showing ESD clamping arrays & series current limiters on every node]
@DEV: You don't make the bus gentle. You make every node indestructible.
# （バスを優しく設計するんじゃない。全ノードを壊れないように武装させるんだ。）

@DEV: We made every single line on the IDC bus 3V3-tolerant by putting dedicated ESD clamping arrays and series current limiters on every node. Even if a 12-volt rail shorts directly into the bus, current is capped at under ten milliamps.
# （全ノードに専用のESDクランプアレイと直列制限抵抗を放り込んで、IDCバスの全ラインを3V3トレラントにした。たとえ12Vラインがバスに直撃しても、電流は10ミリアンペア以下に抑え込まれる。）

@DEV: Let's check the numbers. Clamping response: three point eight nanoseconds. Current dissipation: safely within package thermal limits. The diodes eat the spike, and the microcontroller doesn't even flinch.
# （数字を見てみよう。クランプ応答は3.8ナノ秒。消費電力はパッケージの熱許容量に余裕で収まる。ダイオードが雷を丸呑みして、マイコンは眉ひとつ動かさない。）

# [PAUSE 0.5s]
@DEV: Think about what that means. Instead of designing a polite, fragile bus and praying nobody makes a mistake, I basically gave every microcontroller on the board a bomb shelter and a riot shield.
# （つまりこういうことだ。行儀よくて壊れやすいバスを設計して「誰もミスしませんように」と祈る代わりに、基板上の全マイコンに核シェルターと暴徒鎮圧用シールドを配備したわけだ。）

@DEV: Is it elegant? Not even close. It is pure, shameless brute force in silicon.
# （エレガントかって？ 程遠いね。シリコンの壁を使った、純粋で恥知らずな力押しだ。）

@DEV: But when your factory failure rate drops from fourteen percent to zero on a single cable click? Brute force that works is just called engineering.
# （だが、ケーブル1本挿すだけで工場の不良率が14%からゼロに落ちるなら？ 動く力押しは、ただ「エンジニアリング」と呼ぶんだよ。）
