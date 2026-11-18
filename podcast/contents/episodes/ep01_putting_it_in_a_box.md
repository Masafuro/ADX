# Episode 01: Why Build Another MCU Board When the $4 Pico Exists? (What It Means to Put a Board in a Box)
# Off the Breadboard — Stage 2 Production Script (Converted from drafts/ep01_stage1_enclosure.md)

# [VISUAL: Arduino UNO and Raspberry Pi Pico next to an ADX PCB and a sealed 3D-printed enclosure]
@DEV: We live in a world with the Arduino UNO and the four-dollar Raspberry Pi Pico. They are cheap, battle-tested, and sitting in stock everywhere on Earth.
# （世の中には、Arduino UNOやたった4ドルのRaspberry Pi Picoがある。安くて実績もあって、地球上のどこでも買える。）

@DEV: So if you look at what we're building with ADX, you're probably asking the most obvious question possible: why on earth am I burning months designing my own microcontroller boards from scratch?
# （だから俺たちがADXで作っているものを見たら、きっと一番真っ当な疑問が浮かぶはずだ。「いったい何のために、何ヶ月もかけてわざわざ一から自作マイコンボードなんて設計してるんだ？」と。）

# [PAUSE 0.4s]
@DEV: Here is the short answer. An Arduino UNO and a Pico are designed to be heroes on a breadboard. The second you try to put them inside a box, they start fighting you.
# （短い答えはこうだ。Arduino UNOもPicoも、ブレッドボードの上で英雄になるように設計されている。だが箱に入れようとした瞬間、あいつらは牙を剥いて抵抗してくるんだ。）

# [PAUSE 0.4s]
@DEV: Now, I know what you're thinking. "Wait—it's just a circuit board! You 3D-print four plastic walls, screw the board to the floor, cut a hole for the USB cable, and snap the lid shut. How hard can putting something in a box possibly be?"
# （今あんたが思ってることは分かってる。「待てよ、ただの基板だろ！ プラスチックの壁を4面3Dプリントして、底にネジ止めして、USB用の穴を開けて、蓋をパチンと閉めるだけじゃないか。箱に入れるだけの何がそんなに難しいんだ？」ってな。）

@DEV: That's what everyone thinks—until they actually try to seal the lid. Let me walk you through what happens the moment an off-the-shelf dev board goes inside a real enclosure.
# （蓋を閉めるまでは誰もがそう思う。市販の開発ボードを本物の筐体に押し込んだ瞬間に何が起きるか、順を追って見てみよう。）

# [VISUAL: Hot-glued dev board inside a plastic case & loose Dupont jumper wires]
@DEV: First: where are the mounting holes? On half of these boards, mounting holes are an afterthought—or missing completely. That's why half the projects on the internet end up hot-gluing a microcontroller straight to the bottom of a plastic case.
# （まず1つ目、固定穴はどこにある？ 市販ボードの半分は、ネジ穴が後付けか、そもそも無い。だからこそ、ネット上の工作の半分は、プラスチックケースの底にマイコンをグルーガンで直貼りする羽目になる。）

@DEV: Second: look at the wires. On a breadboard, Dupont jumper wires look fine. Put them inside a dark box that gets moved, bumped, or mounted on a wall—and one loose jumper wire kills the whole project. Nobody can see it, because the lid is closed.
# （2つ目、配線を見ろ。ブレッドボードの上ならDupontのジャンパ線でも問題ない。だが、動かされたり、ぶつけられたり、壁に取り付けられたりする暗い箱の中に入れた瞬間、たった1本の緩んだジャンパ線が作品全体を殺す。しかも蓋が閉まっているから、誰にも見えない。）

# [PAUSE 0.4s]
@DEV: Sure, maybe you're thinking: "Okay, I'll solder everything directly or use screw terminals." Great. Then tell me what you do when the firmware hangs.
# （もちろん、あんたはこう言うかもしれない。「よし、じゃあ全部直付けはんだするか、ネジ端子を使えばいい」と。いいだろう。じゃあ、ファームウェアが固まったときどうする？）

# [VISUAL: Sealed 4-screw box hiding a tiny BOOTSEL / RESET tactile switch]
@DEV: On your desk, the Pico's BOOTSEL button or the Arduino's RESET button sits two inches from your finger. Once you tighten four screws on an enclosure, that tiny tactile button is locked inside a sealed tomb. Now you're grabbing a screwdriver just to reflash a chip. Or what—are you going to drill a pinhole in the side of the box and poke it with a paperclip? Very industrial.
# （机の上なら、PicoのBOOTSELボタンもArduinoのRESETボタンも、あんたの指から2インチの距離にある。だが筐体の4本のネジを締めた瞬間、あの小さなタクトスイッチは密閉された墓場の中に閉じ込められる。チップを書き換えるためだけに、毎回ドライバーを握る羽目になるんだ。それとも何かい、箱の横に極小の穴を開けて、クリップの先で突っつくのか？ 実に産業グレードだな。）

# [PAUSE 0.5s]
@DEV: And we haven't even touched power and heat yet. On your workbench, your board pulls five hundred milliamps from a laptop USB port and sits in open air. Seal a high-clock wireless chip inside a tight PLA case, and suddenly the air stops moving. The plastic softens, and your temperature sensor starts reading three degrees hotter than the room—because the microcontroller is literally cooking its own house.
# （しかも、電源と熱の話にはまだ触れてすらいない。作業机の上では、ボードはノートPCのUSBポートから500mAを引っ張り、風通しのいい空間に置かれている。だが高クロックの無線チップを密閉PLAケースに閉じ込めた瞬間、空気はピタッと止まる。プラスチックは軟化し、温度センサーは室温より3℃高い嘘を吐き始める――マイコンが文字通り自分の家を温めているからだ。）

# [PAUSE 0.6s]
@DEV: Think about what that means. On a breadboard, a microcontroller lives in a forgiving greenhouse: infinite airflow, visible wires, and your own finger right next to the reset button. The second you put it in a box, every single one of those assumptions vanishes.
# （つまりこういうことだ。ブレッドボードの上では、マイコンはすべてが許される温室に生きている――無限の空気、見える配線、リセットボタンのすぐ横にあるあんたの指。箱に入れた瞬間、その前提はひとつ残らず消滅する。）

# [VISUAL: Conceptual breakdown — Protection / Environment / Human Interface]
@DEV: Because putting a board in a box is the exact moment a hobby experiment stops being yours and has to become a tool.
# （なぜなら、基板を箱に入れるという行為は、ホビーの実験が「自分の手元のおもちゃ」であることをやめて、ひとつの「道具」にならなければならないまさにその瞬間だからだ。）

@DEV: A box isn't just four walls. It's protection from dust and hands. It's adaptation to a messy physical world. And most of all, it's a physical interface for a human being who didn't wire the circuit—someone who just expects it to work when they plug it in.
# （箱はただの4枚の壁じゃない。埃や人間の手からの保護であり、過酷な物理世界への適応だ。そして何より、回路を組んでいない普通の人間――「挿せば動くはずだ」と期待する誰か――と向き合うための物理インターフェースそのものなんだ。）

# [PAUSE 0.6s]
@DEV: If you look through our commit history on ADX, you'll see us changing board dimensions, arguing over one-millimeter screw margins, stripping buzzers off boards, and rewriting specs over and over. From the outside, it probably looks chaotic.
# （ADXのコミット履歴を覗けば、俺たちが基板のサイズを変え、1ミリのネジ余白で悩み、ボードからブザーを削ぎ落とし、何度も仕様を書き直しているのが見えるはずだ。外から見たら、迷走しているように見えるかもしれない。）

@DEV: And if you ask me whether we have finally found the neat, perfect answer for "the box"? Honestly? No. Every time we solve one enclosure problem, the real world hands us a new one. I still don't have all the answers.
# （それで、もし「結局、箱に入れることへの綺麗で完璧な答えは見つかったのか？」と訊かれたら？ 正直に言うと、まだだ。筐体の問題を1つ解決するたびに、現実世界がまた新しい課題を突きつけてくる。俺もまだ、すべての答えを持っているわけじゃない。）

# [PAUSE 0.5s]
@DEV: That is the quiet truth behind ADX. We aren't trying to beat a Raspberry Pi Pico on a spec sheet. We're building an entire hardware standard around a question we still haven't finished answering: how do electronics survive once they leave the breadboard?
# （それがADXの背後にある静かな真実だ。俺たちはスペック表の上でRaspberry Pi Picoに勝とうとしているんじゃない。まだ答えを出しきれていないひとつの問い――「電子回路は、ブレッドボードを離れたあとどうやって生き残るのか？」――その問いを中心に、ハードウェア規格を丸ごと作っているんだ。）

@DEV: Welcome to Off the Breadboard. Let's see what breaks next.
# （『Off the Breadboard』へようこそ。さあ、次はどこが壊れるか見ていこうじゃないか。）
