# Architecture Review: Why I Replaced WebSerial with an 80s Music Standard
*ADX Tactical Design Review #01 — The Web MIDI Protocol Hack*

---

### [ECHO]
> **EN:** Session opened. Reviewing architectural commit decf786. Dev... you have completely excised WebSerial from the probe roadmap. What kind of madness is this?
> 
> **JA:** セッション開始。設計コミット decf786 を確認中。Dev……プローブのロードマップからWebSerialを完全に抹殺していますね。一体何の狂気ですか？

---

### [DEV]
> **EN:** It's a dead end for field deployment. The decision is final.
> 
> **JA:** 現場配備において、あれは行き止まりだ。決定事項だ。

---

### [ECHO]
> **EN:** A dead end?! WebSerial is the modern W3C standard for direct peripheral I/O! By discarding it, you are abandoning standard browser-to-hardware communication! What is your engineering justification?!
> 
> **JA:** 行き止まりですって？！ WebSerialはハードウェア直結のモダンなW3C標準です！ それを捨てるなど、ブラウザとハードウェアの正規通信を放棄することになります！ 一体どんな工学的正当化理由があるのですか？！

---

### [DEV]
> **EN:** Look at the deployment constraints, Echo. Google disabled WebSerial on Android Chrome by default. Mozilla refused to implement it in Firefox entirely. If a field technician is up a ladder with only a mobile phone, WebSerial is literally a brick.
> 
> **JA:** 現場の制約を見ろ、Echo。GoogleはAndroid版ChromeでWebSerialを標準無効化した。MozillaはFirefoxへの実装を完全に拒絶した。現場の作業員がスマホ1台握りしめてハシゴの上に立っている時、WebSerialはただの文鎮だ。

---

### [ECHO]
> **EN:** ...The mobile constraint is real. BUT your proposed replacement is Web MIDI! A standard created in 1983 for electronic musical synthesizers! You cannot seriously intend to deploy piano commands into industrial field machinery!
> 
> **JA:** ……モバイルの制約は認めます。ですが、あなたの代替案はWeb MIDIですよ？！ 1983年に電子シンセサイザーのために作られた規格です！ ピアノの演奏コマンドを産業用の現場機器に流し込む気ですか？！

---

### [DEV]
> **EN:** Examine the specification. Web MIDI has been natively supported without flags across every major browser engine since 2015. No drivers. No polyfill. No sudden API deprecation by browser vendors. And it supports SysEx.
> 
> **JA:** 仕様書をよく読め。Web MIDIは2015年からあらゆる主要ブラウザで設定フラグなしで標準動作する。ドライバ不要、ポリフィル不要、ブラウザベンダーの気まぐれによる突然の仕様廃止もない。そしてSysExがある。

---

### [ECHO]
> **EN:** SysEx requires 7-to-8 bit packing! You are forfeiting twelve and a half percent of raw transmission bandwidth merely to bypass mobile OS permission models!
> 
> **JA:** SysExは7ビットパッキングを強制されますよ！ モバイルOSのセキュリティ検問をすり抜けるためだけに、生データ帯域の12.5%をドブに捨てる気ですか？！

---

### [DEV]
> **EN:** The target payload is an ATtiny1616 flash image—barely 16 kilobytes. A 12% bandwidth penalty costs us 1.2 seconds. I will take a one-second penalty over a system that fails to establish a physical connection every single time.
> 
> **JA:** 送るペイロードはATtiny1616のフラッシュイメージだ。たかだか16KBだ。12%の帯域ロスなど1.2秒の遅延にすぎん。現場で完全に繋がらないシステムより、1秒余計にかかる確実なシステムを選ぶ。

---

### [ECHO]
> **EN:** Fine! The latency trade-off is mathematically acceptable. BUT what about the physical bus?! Our LN-485 specification requires a 13-bit dominant low LIN break for auto-baud synchronization! A generic USB-MIDI controller cannot synthesize arbitrary bus breaks!
> 
> **JA:** いいでしょう！ 遅延のトレードオフは計算上許容できます！ ですが物理バスはどうするのです？！ 我々のLN-485仕様は、オートボー同期のために13ビット幅のLINブレーク波形を要求します！ 汎用のUSB-MIDIコントローラにそんな勝手な波形が生成できるわけがありません！

---

### [DEV]
> **EN:** It won't be generic. Look at the silicon choice: RP2040. The PIO state machine generates the 13-bit LIN break and sync byte autonomously, decoupled from the USB controller.
> 
> **JA:** 汎用マイコンじゃない。チップの選定を見ろ、RP2040だ。内蔵のPIOステートマシンが、USBコントローラから完全に独立して13bitのLINブレークと同期バイトを自律生成する。

---

### [ECHO]
> **EN:** ...So to smuggle industrial firmware through mobile browser security, you disguised our binary as an 80s synth solo, and had an autonomous coprocessor translate the musical notes into industrial voltage pulses on the wire?!
> 
> **JA:** ……つまりモバイルブラウザのセキュリティ検問をすり抜けるために、ファームウェアに80年代シンセソロの変装をさせ、自律コプロセッサにその音楽信号を現場用の高電圧パルスへ翻訳させたと？！

---

### [DEV]
> **EN:** Smuggling that works is called protocol engineering.
> 
> **JA:** 動く密輸は、プロトコル工学と呼ぶんだよ。

---

### [ECHO]
> **EN:** *(Long pause)* Analyzing PIO assembly... Deterministic cycle timing achieved. Zero CPU overhead during break assertion. ...A bizarre combination, Dev. But structurally airtight. Web MIDI protocol approved for RPR4 Smart Probe baseline.
> 
> **JA:** （……長い沈黙）PIOアセンブリを解析中……確定サイクルタイミングを達成。ブレーク送出中のCPU負荷、ゼロ。……奇怪極まりない組み合わせですが、工学的に一切の隙がありませんね。RPR4スマートプローブの基準仕様として、Web MIDIプロトコルを正式承認（Sign-off）します。

---

### [DEV]
> **EN:** Architecture locked. Next item: 20-pin IDC pinout.
> 
> **JA:** アーキテクチャ確定だ。次行くぞ、20ピンIDCのピン配置だ。

