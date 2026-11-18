# Episode 01: Why I Replaced WebSerial With a 1983 Music Standard
# v3 Pilot Script — DEV First-Person Monologue

# [VISUAL: Git diff highlighting WebSerial removal and Web MIDI SysEx addition on RPR4]
@DEV: I just ripped WebSerial out of our industrial field programmer and replaced it with MIDI. Yes—the 1983 synthesizer protocol.
# （産業用の現場プログラマからWebSerialを引っこ抜いて、MIDIに差し替えた。そう、あの1983年のシンセサイザー規格だ。）

# [PAUSE 0.4s]
@DEV: Look, I know how that sounds. When I pushed the commit and ran it past Echo—our automated design-review AI—it flagged the branch almost immediately.
# （いや、どう聞こえるかは分かってる。このコミットをプッシュして、設計レビュー用のAI『Echo』にかけた瞬間、即座に警告フラグが立った。）

# [HUD: ECHO // AUDIT ALERT: "DEPRECATED LOGIC DETECTED — 12.5% SYSEX OVERHEAD"]
@DEV: Echo's reaction was basically a polite meltdown. "WebSerial is the modern W3C standard for direct hardware I/O," it reminded me. "You cannot seriously intend to flash industrial microcontrollers by sending piano commands over System Exclusive."
# （Echoの反応は、一言でいえば『丁寧な発狂』だった。「WebSerialはハードウェア直結のモダンなW3C標準です。System Exclusiveでピアノの演奏コマンドを送って産業用マイコンを焼く気ですか？」とな。）

@DEV: And honestly? Every objection it threw at me was technically true. First, Web MIDI doesn't let you stream raw binary freely. To get firmware images through SysEx, you have to pack 8-bit data into 7-bit MIDI bytes. That throws away twelve and a half percent of your raw bandwidth right off the top.
# （正直なところ、AIが並べてきたツッコミは全部正論だった。まず、Web MIDIは生のバイナリを自由には流せない。SysEx経由でファームウェアを送るには、8ビットのデータを7ビットのMIDIバイトに詰め直す必要がある。その時点で生帯域の12.5%が消える。）

# [VISUAL: LN-485 13-bit LIN break timing diagram]
@DEV: Second, our RS-485 bus protocol needs a very specific thirteen-bit dominant-low LIN break pulse just to wake up the target chips and sync the baud rate. A standard USB-MIDI controller plays notes. It doesn't hold an industrial serial line low on microsecond command.
# （第二に、うちのRS-485バスプロトコルは、ターゲットのマイコンを起こしてボーレートを同期させるためだけに、きっちり13ビット幅のドミナントLowによるLINブレーク波形を要求する。普通のUSB-MIDIコントローラは音符を鳴らす道具であって、マイクロ秒単位で産業用シリアル線をLowに引きずる機械じゃない。）

# [PAUSE 0.6s]
@DEV: So on paper, Echo was right. My commit looked like a meme branch.
# （だから机の上の理屈では、Echoが100%正しかった。俺のコミットはネタ用ブランチにしか見えなかったはずだ。）

# [VISUAL: Browser support matrix — Android Chrome & Firefox marked red for WebSerial]
@DEV: Here is what the spec sheet doesn't tell you, though. Imagine a field technician standing halfway up a ladder in a factory, holding nothing but an Android phone to reflash a dead sensor node.
# （だが、仕様書が教えてくれない現実がひとつある。工場のハシゴの途中で、死んだセンサーノードを復旧させようとAndroidスマホ1台だけを握りしめている作業員を想像してみてほしい。）

@DEV: Google disabled WebSerial on Android Chrome by default. Mozilla refused to implement it in Firefox altogether. That means under the clean, modern W3C standard, that technician's phone is a brick. You don't lose twelve percent of your bandwidth—you lose a hundred percent of your connection.
# （GoogleはAndroid版ChromeでWebSerialを標準無効にした。MozillaはFirefoxへの実装自体を拒否した。つまり、綺麗でモダンなW3C標準に従う限り、その作業員のスマホはただの文鎮になる。帯域を12%失うどころか、接続そのものを100%失うんだ。）

@DEV: Now look at Web MIDI. It has been supported natively without flags across major browsers since 2015. No drivers, no polyfills, and no OS vendor suddenly pulling the plug on the API.
# （そこでWeb MIDIを見てみる。2015年から主要ブラウザで設定フラグなしに標準対応している。ドライバもポリフィルも不要で、ベンダーの都合でAPIが突然死ぬ心配も薄い。）

# [PAUSE 0.5s]
@DEV: And what about that twelve-percent bandwidth penalty Echo complained about? Our target MCU is an ATtiny1616. Its entire flash memory is sixteen kilobytes. At our bus speed, seven-to-eight packing adds one point two seconds to an upload. I will gladly trade one second for a cable that actually talks to a phone.
# （じゃあEchoが文句を言っていた12%の帯域ロスはどうなのか？ ターゲットのMCUはATtiny1616。フラッシュメモリ全体でたった16キロバイトだ。うちのバス速度で7-to-8パッキングをしても、書き込みが増える時間は1.2秒。現場のスマホと確実に繋がるなら、1秒くらい喜んで差し出すさ。）

# [VISUAL: RPR4 Smart Probe architecture — RP2040 USB-MIDI + PIO LIN break generator]
@DEV: That left only one real engineering problem: the thirteen-bit LIN break. And the fix wasn't software—it was silicon. Instead of a generic USB-MIDI chip, the RPR4 probe uses an RP2040.
# （これで残った課題は1つだけ。13ビットのLINブレークだ。そして解決策はソフトじゃなくシリコンだった。RPR4プローブは汎用のUSB-MIDIチップじゃなく、RP2040を載せている。）

@DEV: While the USB stack unpacks SysEx messages from the browser, one of the RP2040's independent PIO state machines sits right next to the RS-485 transceiver and bit-bangs the exact thirteen-bit break and sync byte autonomously, with zero CPU overhead.
# （USBスタックがブラウザからのSysExメッセージを解凍している裏で、RP2040の独立したPIOステートマシンの1つがRS-485トランシーバの真横に陣取り、CPU負荷ゼロで精密な13ビットブレークと同期バイトを自律的に叩き出している。）

@DEV: Essentially, I disguised industrial firmware as an eighties synth solo to sneak it past mobile browser security, and hired a tiny hardware coprocessor to translate the sheet music back into industrial voltage pulses on the wire.
# （要するに、モバイルブラウザの検問をすり抜けるために産業用ファームウェアに80年代シンセソロの変装をさせ、小さなハードウェア・コプロセッサを雇って、その楽譜を現場用の電圧パルスに翻訳し直しているわけだ。）

# [PAUSE 0.6s]
# [HUD: ECHO // STATUS UPDATED: "TIMING DETERMINISTIC // SPEC APPROVED (RELUCTANTLY)"]
@DEV: When I fed the PIO timing numbers back to Echo, it went quiet for a few seconds, verified the cycle count, and reluctantly cleared the spec. It still hates that our industrial field probe shows up in Device Manager as a musical instrument.
# （PIOのタイミング数値をEchoに食わせ直したら、数秒黙り込んでサイクル数を検証したあと、渋々仕様を承認した。いまだにうちの産業用プローブがデバイスマネージャ上で楽器として認識されることには文句を垂れているがな。）

@DEV: Ugly? Sure. But it works on a ladder with a phone—and in hardware, shipping something that works beats being right on paper every single time.
# （美しくないかって？ そりゃそうだ。だがハシゴの上のスマホで動く。ハードウェアの世界では、机の上で正しいことより現場で動くものがいつだって勝つんだ。）
