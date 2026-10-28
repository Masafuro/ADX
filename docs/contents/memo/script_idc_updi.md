# Why I Put 12V High-Voltage UPDI into a 20-Pin Ribbon Cable
*ADX Tactical Design Review #02 — Manufacturing Yield vs. Bus Safety*

---

### [ECHO]
> **EN:** Dev. I just pulled your latest commit. ...Are you attempting to commit arson, or did you accidentally push a meme branch?
> 
> **JA:** Dev。最新のコミットを取得しました。……放火予告ですか？ それとも間違えてネタ用のブランチをプッシュしたのですか？

---

### [DEV]
> **EN:** I put UPDI on Pin 14 of the IDC bus. Problem solved.
> 
> **JA:** IDCバスのピン14にUPDIを入れた。これで問題解決だ。

---

### [ECHO]
> **EN:** 'Problem solved'?! Pin 14 is inside a flat ribbon cable with zero shielding! Microchip's UPDI recovery requires TWELVE VOLTS! You are routing a miniature lightning bolt directly adjacent to fragile 3.3-volt logic lines! Did your biological processor suffer an electrostatic discharge?!
> 
> **JA:** 『問題解決』ですって？！ ピン14はシールド無しのフラットリボンケーブルの中ですよ！ MicrochipのUPDI復旧には12ボルトが必要です！ 繊細な3.3Vロジックの真隣に、ミニチュアの雷を走らせる気ですか！ あなたの生体プロセッサは静電気でショートしたのですか？！

---

### [DEV]
> **EN:** High voltage builds character. Besides, the HV pulse is only a few microseconds. The chips will barely feel it.
> 
> **JA:** 高電圧はマイコンを強くする。それに、HVパルスは数マイクロ秒だ。チップも痛がりゃしない。

---

### [ECHO]
> **EN:** Silicon does not 'build character', Dev! It releases magic smoke and dies! For three months, you swore that high voltage would never touch the IDC bus. What insanity possessed you to reverse that policy?
> 
> **JA:** シリコンは強くなんてなりません、Dev！ 魔法の煙を吹いて死ぬんです！ 高電圧は絶対にIDCバスに通さないと3ヶ月間誓っていたはずです。一体どんな狂気に取り憑かれて方針をひっくり返したのですか？

---

### [DEV]
> **EN:** Reality possessed me, Echo. Look at the factory floor. We're using ATtiny1616 in 3-millimeter VQFNs on OSP boards. Have you ever tried using a bed-of-nails pogo jig on oxidized OSP pads?
> 
> **JA:** 現実だよ、Echo。工場の現場を見ろ。俺たちはOSP基板に3mmの微細VQFNを載せてるんだ。酸化したOSPパッドに、針山のポゴピン治具を当てたことがあるか？

---

### [ECHO]
> **EN:** ...Analyzing production logs. Contact resistance spikes... 14% false verification failure rate... manual rework required.
> 
> **JA:** ……生産ログを解析中。接触抵抗のスパイク……14%の書き込み偽エラー……手動リワークの発生……

---

### [DEV]
> **EN:** A bed-of-nails fixture is a medieval torture device. Pins bend, alignment drifts, and the operators want to kill me. With UPDI on the IDC cable, we plug in ONE ribbon cable. Flash the bootloader, run the loopback tests, verify the transceiver, and lock the chip. Zero pogo pins. One click. Done.
> 
> **JA:** 針山治具なんて中世の拷問器具だ。ピンは曲がる、位置はズレる、現場の作業員は俺を殺しそうな目で見てくる。IDCにUPDIを載せれば、リボンケーブルを1本カチッと挿すだけだ。ブートローダー書き込み、ループバック検査、トランシーバ試験、ヒューズロック。針山ゼロ。ワンクリックで終わりだ。

---

### [ECHO]
> **EN:** I admit that mechanical pogo fixtures are a nightmare. AND the single-insertion test sequence is undeniably efficient. BUT THAT DOES NOT CHANGE PHYSICS, DEV! If someone accidentally triggers a 12V pulse while a standard card is connected, the entire bus is incinerated!
> 
> **JA:** 針山治具が悪夢だということは認めます。ケーブル1本でのテスト工程が圧倒的に効率的なのも認めます。でも、物理法則は変わらないんですよ、Dev！ 標準カードが繋がっている時に誰かが誤って12Vパルスを撃ったら、バス全体が一瞬で消し炭です！

---

### [DEV]
> **EN:** Which is why I made every single line on the IDC bus 3V3-tolerant. Look at the schematic. Dedicated ESD clamping arrays and series current limiters on every node. Even if the 12V rail shorts directly into the bus, current is capped at under 10 milliamps. The diodes eat the spike. The MCU doesn't even flinch.
> 
> **JA:** だからIDCバスの全ラインを3V3トレラントにしたんだ。回路図を見ろ。全ノードに専用ESDクランプアレイと直列制限抵抗をブチ込んだ。たとえ12Vがバスに直撃しても、電流は10mA以下に抑え込まれる。ダイオードが雷を丸呑みする。MCUは眉ひとつ動かさない。

---

### [ECHO]
> **EN:** ...Clamping response: 3.8 nanoseconds. Current dissipation within package thermal limits. ...So instead of designing a gentle bus, you armed every single microcontroller with a bomb shelter and a riot shield?! You literally brute-forced a safety margin using a wall of silicon.
> 
> **JA:** ……クランプ応答：3.8ナノ秒。パッケージの熱許容量以内。……つまり安全で大人しいバスを設計する代わりに、全マイコンに核シェルターと暴徒鎮圧用シールドを配備したってことですか？！ あなた、シリコンの壁で物理的に安全マージンを力押しでねじ込みましたね。

---

### [DEV]
> **EN:** Brute force that works is called engineering.
> 
> **JA:** 動く力押しは、工学（エンジニアリング）と呼ぶんだよ。

---

### [ECHO]
> **EN:** *(Heavily sighs)* I hate that your math actually checks out. Fine. Yield wins over elegance. Pin 14 is locked for UPDI. ADX Bus Specification Rev 2.1 is officially signed-off. ...Just keep your 12-volt lightning away from my diagnostic port.
> 
> **JA:** （……重いため息）計算が合っているのが本当に腹立たしいですね。いいでしょう。優雅さよりも歩留まりの勝利です。ピン14のUPDI割り当てを確定。ADXバス仕様書Rev 2.1を正式承認（Sign-off）します。……ただし、その12ボルトの稲妻を私の診断ポートに近づけないでくださいね。

---

### [DEV]
> **EN:** No promises. Next item: CORE-A5 test spec.
> 
> **JA:** 約束はできないな。次だ、CORE-A5のテスト仕様行くぞ。
