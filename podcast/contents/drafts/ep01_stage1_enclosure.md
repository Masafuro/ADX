# Stage 1 Dialectic Draft (v1 Engine): Episode 01
# Theme: Arduino UNO / Raspberry Pi Pico があるのに、なぜわざわざ自作マイコンボードを作るのか？（＝「箱に入れる」とはどういうことか？）
# Source Memo: podcast/contents/memo/0001_箱に入れるとはどういうことか？

@ECHO: Dev. I need a sanity check on the entire ADX repository. We live in a world with the Arduino UNO and the four-dollar Raspberry Pi Pico. They are cheap, battle-tested, and sitting in stock everywhere on Earth. Why on earth are you burning months designing your own microcontroller boards from scratch?
# （Dev、ADXリポジトリ全体の正気度チェックが必要です。世の中にはArduino UNOや、たった4ドルのRaspberry Pi Picoがあるんですよ。安くて実績もあって、地球上のどこでも買える。それなのに、なぜ何ヶ月もかけてわざわざ一から自作マイコンボードなんて設計しているんですか？）

@DEV: Because an Arduino UNO and a Pico are designed to be heroes on a breadboard. The second you try to put them inside a box, they start fighting you.
# （Arduino UNOもPicoも、ブレッドボードの上で英雄になるように設計されてるからだ。箱に入れようとした瞬間、あいつらは牙を剥いて抵抗してくる。）

@ECHO: "Fighting you"?! Dev, it is a circuit board! You 3D-print four plastic walls, screw the board to the floor, cut a hole for the USB cable, and snap the lid shut! How hard can putting something in a box possibly be?!
# （「抵抗してくる」ですって？！ Dev、ただの基板ですよ！ プラスチックの壁を4面3Dプリントして、底にネジ止めして、USB用の穴を開けて、蓋をパチンと閉めるだけじゃないですか！ 箱に入れるだけの何がそんなに難しいんですか？！）

@DEV: That's what everyone thinks until they actually try to seal the lid. Look at what happens when an off-the-shelf dev board goes inside a real enclosure.
# （蓋を閉めるまでは誰もがそう思うんだよ。市販の開発ボードを本物の筐体に押し込んだ瞬間に何が起きるか見てみろ。）

@DEV: First: where are the mounting holes? On half these boards they're an afterthought—or missing completely. Makers end up hot-gluing a microcontroller to the bottom of a plastic case.
# （まず1つ目、固定穴はどこにある？ 市販ボードの半分はネジ穴が後付けか、そもそも無い。結果、メイカーたちはマイコンをプラスチックケースの底にグルーガンで接着する羽目になる。）

@DEV: Second: look at the wires. On a breadboard, Dupont jumper wires look fine. Put them inside a dark box that gets moved, bumped, or mounted on a wall—and one loose jumper wire kills the whole project. Nobody can see it, because the lid is closed.
# （2つ目、配線を見ろ。ブレッドボードの上ならDupontのジャンパ線でも問題ない。だが、動かされたり、ぶつけられたり、壁に取り付けられたりする暗い箱の中に入れた瞬間、たった1本の緩んだジャンパ線が作品全体を殺す。しかも蓋が閉まってるから誰も見えない。）

@ECHO: Okay, fine—use screw terminals or solder the wires directly! Still doesn't explain why you need an entirely new board architecture!
# （分かりました、じゃあネジ端子を使うか直付けはんだすればいいじゃないですか！ それでも新しい基板アーキテクチャを丸ごと作る理由にはなりません！）

@DEV: Then explain what you do when the firmware hangs. On your desk, the Raspberry Pi Pico's BOOTSEL button or the Arduino's RESET button is sitting two inches from your finger. Once you tighten four screws on an enclosure, that tiny tactile button is trapped inside a sealed tomb. You're grabbing a screwdriver just to reflash a chip.
# （じゃあファームウェアが固まったときどうするんだ？ 机の上なら、Raspberry Pi PicoのBOOTSELボタンもArduinoのRESETボタンも指から2インチの位置にある。だが筐体の4本のネジを締めた瞬間、あの小さなタクトスイッチは密閉された墓場の中に閉じ込められる。チップを書き換えるためだけに毎回ドライバーを握る羽目になるぞ。）

@ECHO: ...You could drill a pinhole in the case and poke it with a paperclip?
# （……ケースに極小の穴を開けて、クリップの先で突っつくとか？）

@DEV: Right. Very industrial. And what about power and heat? On a workbench, your board pulls 500 milliamps from a laptop USB port and sits in open air. Seal a high-clock wireless chip inside a tight PLA box, and suddenly the air doesn't move, the case softens, and your temperature sensor reads three degrees hotter than the room because the MCU is cooking its own house.
# （そうだな、実に産業グレードだ。それで、電源と熱はどうする？ 作業机の上では、ノートPCのUSBポートから500mAを引っ張り、風通しのいい空間に置かれている。だが高クロックの無線チップを密閉PLAケースに閉じ込めた瞬間、空気は動かなくなり、ケースは歪み、温度センサーはマイコンが自分の家を温めているせいで室温より3℃も高い嘘を吐き始める。）

@ECHO: ...So on a breadboard, a microcontroller lives in a forgiving greenhouse—infinite airflow, visible wires, and a human finger right next to the reset button. Inside a box, all of those assumptions vanish?
# （……つまりブレッドボードの上では、マイコンはすべてが許される温室に生きている――無限の空気、見える配線、リセットボタンのすぐ横にある人間の指。でも箱の中では、その前提がすべて消滅するということですか？）

@DEV: Worse than that. Putting a board in a box is the exact moment a hobby experiment stops being yours and becomes a tool. A box is protection from dust and hands. It's adaptation to a messy world. And it's a physical interface for a human being who didn't wire the circuit and just expects it to work when they plug it in.
# （それだけじゃない。基板を箱に入れるという行為は、ホビーの実験が「自分の手元のおもちゃ」であることをやめて、ひとつの「道具」に変わる瞬間なんだ。箱は埃や手からの保護であり、過酷な現実への適応であり、回路を組んでいない普通の人間が「挿せば動くはずだ」と期待する物理インターフェースそのものなんだよ。）

@ECHO: Wait a second... Let me check your commit history again. You've changed board dimensions, argued over one-millimeter screw margins, stripped buzzers off boards, and rewritten specs over and over. If Arduino and Pico are the wrong starting point... have you actually found the right answer for "the box" yet?
# （ちょっと待ってください……コミット履歴を再確認します。あなた、基板サイズを変え、1ミリのネジ余白で悩み、ボードからブザーを削ぎ落とし、何度も仕様を書き直していますよね。ArduinoやPicoが出発点として間違っていたとして……あなたは結局、「箱に入れる」ことへの正解を見つけたんですか？）

@DEV: Honestly? No. Not completely. Every time we solve one box problem, the physical world hands us another one. I still don't have a neat, final answer.
# （正直に言うと？ いや。まだ完全には見つかっていない。箱の問題を1つ解決するたびに、物理世界がまた別の課題を突きつけてくる。綺麗な最終回答なんて、まだ持っていないんだ。）

@ECHO: ...Let me get this straight. You aren't building a better Raspberry Pi Pico. You're building an entire hardware standard around a question you still haven't fully answered?
# （……整理させてください。あなたは「より高性能なRaspberry Pi Pico」を作っているわけではない。自分がまだ答えを出しきれていない「箱に入れるとはどういうことか」という問いそのものを中心に、ハードウェア規格を丸ごと作っているのですか？）

@DEV: Now you get why ADX has been struggling from day one. We're not competing with dev boards on a spec sheet. We're trying to figure out how electronics survive once they leave the breadboard.
# （これでADXが初日からなぜこんなにも苦労しているか分かっただろ。俺たちはスペック表の上で開発ボードと競争してるんじゃない。電子回路がブレッドボードを離れたあと、どうやって生き残るのかを必死に探しているんだよ。）

@ECHO: *(Long pause)* ...You are chasing a problem with no clean textbook answer, using boards you keep tearing apart and rebuilding. ...I hate that this actually makes sense. Log opened. Let's see how badly the box wins next week.
# （（長い沈黙）……教科書に綺麗な答えが載っていない問題を追いかけながら、基板を壊しては作り直しているわけですか。……悔しいですが、それでようやくすべての辻褄が合いました。記録開始。来週は箱にどれだけボコボコにされるか見物ですね。）
