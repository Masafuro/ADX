# Episode 01: Why I Replaced WebSerial With a 1983 Music Standard
# v4 Pilot Script — Direct "I"-to-"You" Engineering Address

# [VISUAL: RPR4 Smart Probe hardware & Web MIDI SysEx block diagram]
@DEV: I just ripped WebSerial out of our industrial field programmer and replaced it with MIDI. Yes—the 1983 synthesizer protocol.
# （産業用の現場プログラマからWebSerialを引っこ抜いて、MIDIに差し替えた。そう、あの1983年のシンセサイザー規格だ。）

# [PAUSE 0.4s]
@DEV: Sounds like a terrible idea, right? Give me three minutes—because by the end of this, you might understand why I had no other choice.
# （最悪のアイデアに聞こえるだろう？ 3分だけくれ。最後まで聴けば、なぜこれしか選択肢がなかったのか分かってもらえるはずだ。）

@DEV: Now, if you've ever built hardware that talks to a browser, I already know the two objections running through your head.
# （さて、ブラウザと通信するハードウェアを作ったことがある人なら、今頭の中に浮かんでいる2つの反論はもうお見通しだ。）

# [VISUAL: SysEx 7-bit packing byte diagram]
@DEV: First, you're thinking about bandwidth. Web MIDI wasn't built to stream arbitrary binary files. To push firmware through a System Exclusive message, you have to pack 8-bit data into 7-bit MIDI bytes. Right out of the gate, you throw away twelve and a half percent of your transmission speed.
# （1つ目は帯域のムダだ。Web MIDIは任意のバイナリファイルを流すようには作られていない。SysExメッセージにファームウェアを通すには、8ビットのデータを7ビットのMIDIバイトに詰め直す必要がある。スタート地点でいきなり通信速度の12.5%を捨てることになる。）

# [VISUAL: LN-485 13-bit dominant low LIN break waveform]
@DEV: And second—maybe the bigger deal-breaker—our RS-485 bus requires a thirteen-bit dominant-low LIN break pulse just to wake the target chips up and lock the baud rate. You know as well as I do: a generic USB-MIDI adapter plays musical notes. It doesn't hold an industrial serial wire low on microsecond command.
# （そして2つ目――こっちのほうが致命的に見えるはずだ――うちのRS-485バスは、ターゲットのマイコンを起こしてボーレートを同期させるためだけに、きっちり13ビット幅のドミナントLowによるLINブレーク波形を要求する。あんたも知っての通り、普通のUSB-MIDIアダプタは音符を鳴らす道具であって、マイクロ秒単位で産業用シリアル線をLowに引きずる機械じゃない。）

# [PAUSE 0.6s]
@DEV: On paper, WebSerial wins that argument ten times out of ten. So why did I kill it?
# （机の上の議論なら、10回やったら10回ともWebSerialの勝ちだ。では、なぜ俺はそれを捨てたのか？）

# [VISUAL: Technician on ladder / Browser support matrix showing Android Chrome & Firefox]
@DEV: Picture yourself on the factory floor. You're standing halfway up a ladder next to a dead sensor node, and all you have in your pocket is an Android phone. You plug in the cable, open the diagnostic page... and nothing happens.
# （自分が工場の現場にいるところを想像してみてほしい。死んだセンサーノードの横、ハシゴの途中に立っていて、ポケットに入っているのはAndroidスマホ1台だけ。ケーブルを挿して、診断ページを開く……だが、何も起きない。）

@DEV: Why? Because Google disabled WebSerial on Android Chrome by default. And Mozilla refused to put it in Firefox at all. Under the clean, modern W3C standard, your phone is a brick. You didn't save twelve percent of your bandwidth—you lost a hundred percent of your connection.
# （なぜか？ GoogleがAndroid版ChromeでWebSerialを標準無効にしたからだ。そしてMozillaはFirefoxへの実装自体を拒絶した。つまり、綺麗でモダンなW3C標準に従う限り、あんたのスマホはただの文鎮になる。帯域の12%を節約できたんじゃない。接続そのものを100%失ったんだ。）

@DEV: Now look at Web MIDI. It has shipped natively, without flags, across major browsers since 2015. No drivers. No polyfills. No waiting on an OS vendor to decide whether you're allowed to talk to a serial port today.
# （一方で、Web MIDIを見てみよう。2015年から主要ブラウザにフラグなしで標準搭載されている。ドライバ不要。ポリフィル不要。OSベンダーが今日シリアルポートを使わせてくれるかどうか気をもむ必要もない。）

# [PAUSE 0.5s]
@DEV: And what about that twelve-percent bandwidth tax? Let's look at the actual numbers. Our target chip, the ATtiny1616, only holds sixteen kilobytes of flash. At our bus speed, 7-to-8 packing costs us one point two seconds per upload. Ask yourself: would you trade one second for a tool that actually works on every phone in your crew's pocket? I took that deal in a heartbeat.
# （じゃあ、さっきの12%の帯域ペナルティはどうなる？ 実際の数字を見よう。ターゲットのATtiny1616のフラッシュ容量はたった16キロバイトだ。うちのバス速度で7-to-8パッキングをしても、書き込みが増える時間はわずか1.2秒。自問してみてほしい。現場の全員のポケットに入っているスマホで確実に動く道具と引き換えに、1秒を払えるか？ 俺は迷わずその取引に乗った。）

# [VISUAL: RP2040 PIO state machine offloading LIN break generation]
@DEV: That leaves just one question: how do you turn MIDI notes into an industrial LIN break? You don't do it with a generic cable—you put the right silicon in the probe.
# （これで残る疑問はたった1つだ。どうやってMIDIの音符を産業用のLINブレークに変えるのか？ 汎用のケーブルでやるんじゃない。プローブの中に適切なシリコンを載せるんだ。）

@DEV: Inside our RPR4 probe sits an RP2040. While the USB side unpacks SysEx messages from your browser, one of the RP2040's independent PIO state machines sits right on the RS-485 transceiver and generates that exact thirteen-bit break and sync byte autonomously, with zero CPU overhead.
# （うちのRPR4プローブの中にはRP2040が入っている。USB側があんたのブラウザから来たSysExメッセージを解凍している裏で、RP2040の独立したPIOステートマシンの1つがRS-485トランシーバの真横に陣取り、CPU負荷ゼロで精密な13ビットブレークと同期バイトを自律生成している。）

@DEV: Think about what that means. To get past mobile browser restrictions, we dressed industrial firmware up like an eighties synth solo, and let a dedicated hardware state machine translate the music back into industrial voltage pulses on the wire.
# （つまりこういうことだ。モバイルブラウザの制限を突破するために、産業用ファームウェアに80年代シンセソロの衣装を着せ、専用のハードウェア・ステートマシンにその音楽を現場の電圧パルスへと翻訳させている。）

# [PAUSE 0.6s]
@DEV: Is it weird that our industrial field probe shows up in Device Manager as a musical instrument? Definitely. Does it feel wrong on paper? Every single time.
# （産業用の現場プローブがデバイスマネージャ上で楽器として認識されるのが変だって？ 間違いなく変だ。書類上は気持ち悪いか？ 毎回気持ち悪いさ。）

@DEV: But engineering isn't about winning beauty contests on a spec sheet. It's about building something that still works when you're halfway up a ladder in the real world.
# （だが、エンジニアリングとはスペック表の美人コンテストで優勝することじゃない。現実の現場で、ハシゴの途中に立っているときでも確実に動くものを作ることなんだ。）
