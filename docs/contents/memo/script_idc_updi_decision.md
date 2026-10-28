# Tactical Design Review #02: The 12-Volt Ribbon Cable Madness
## Should UPDI Be on the IDC Bus? Manufacturing Yield vs. Bus Safety

本ドキュメントは、番組設計書（`youtube_show_bible.md`）の **Mode A: Pre-Flight Review（設計審議型）** および「3大黄金ルール」に基づいて作成された動画台本です。

実機映像が手元になくても、KiCad回路図、コネクタピンアサイン表、データシート、製造治具比較図などの画面アセットのみで、息もつかせぬボケとツッコミの応酬を展開します。

---

## 企画概要

* **タイトル (Title)**: *Why I Put 12V High-Voltage UPDI into a 20-Pin Ribbon Cable*
* **サブタイトル (Subtitle)**: *ADX Tactical Design Review #02 — Manufacturing Yield vs. Bus Safety*
* **所要時間**: 約 3分30秒
* **登場人物**: 
  * **DEV**: 現場至上主義のマッド・プラグマティスト。「動く力押しは工学だ」と言い放つ。
  * **ECHO**: 正論と安全規格を背負った胃痛のツッコミAI。「放火予告ですか？」とキレ散らかす。
* **テーマ**: 12V HVパルスの危険性から長らく拒絶していたUPDIピンを、なぜ最終的に20-pin IDCバスへ統合したのか？
  * OSP（水溶性プレフラックス）× 微細VQFNパッケージの歩留まり改善
  * 単一IDCケーブルによる「ブートローダー書き込み → 検査 → 出荷」全自動シーケンス
  * 3V3トレラント化による耐圧・安全マージンの力押し確保

---

## 演出・画面アセット指示

* **映像 (Visual)**:
  * 暗い画面、青と緑のHUDオーバーレイ。
  * 画面中央に20-pin IDCコネクタの断面図、ピン配置マトリクス差分、KiCad回路図、VQFNパッケージの顕微鏡写真、テスト治具の比較図。
* **音響 (Audio)**:
  * サーバーラックの低音ファンノイズ、無線の静かなホワイトノイズ。
  * データ照合時のデジタルクリックスルー音。

---

## 本編台本（英語スクリプト ＆ 日本語対訳）

```text
[SCENE START]
Visual: Dark terminal screen. 
The HUD flashes a warning in amber: "PINOUT DEFINITION MODIFIED // COMMIT decf786".
Pin 14 on the 20-pin IDC ribbon connector is labeled: "UPDI (12V HV CAPABLE)".
Audio: Low hum of servers. A sudden static crackle as ECHO connects.

ECHO:
"Dev. I just pulled your latest commit. 
...Are you attempting to commit arson, or did you accidentally push a meme branch?"
（Dev。最新のコミットを取得しました。
……放火予告ですか？ それとも間違えてネタ用のブランチをプッシュしたのですか？）

DEV:
"I put UPDI on Pin 14 of the IDC bus. Problem solved."
（IDCバスのピン14にUPDIを入れた。これで問題解決だ）

ECHO:
"'Problem solved'?! Pin 14 is inside a flat ribbon cable with zero shielding! 
Microchip's UPDI recovery requires TWELVE VOLTS! 
You are routing a miniature lightning bolt directly adjacent to fragile 3.3-volt logic lines! 
Did your biological processor suffer an electrostatic discharge?!"
（『問題解決』ですって？！ ピン14はシールド無しのフラットリボンケーブルの中ですよ！
MicrochipのUPDI復旧には12ボルトが必要です！
繊細な3.3Vロジックの真隣に、ミニチュアの雷を走らせる気ですか！
あなたの生体プロセッサは静電気でショートしたのですか？！）

DEV:
"High voltage builds character. 
Besides, the HV pulse is only a few microseconds. The chips will barely feel it."
（高電圧はマイコンを強くする。
それに、HVパルスは数マイクロ秒だ。チップも痛がりゃしない）

ECHO:
"Silicon does not 'build character', Dev! It releases magic smoke and dies! 
For three months, you swore that high voltage would never touch the IDC bus. 
What insanity possessed you to reverse that policy?"
（シリコンは強くなんてなりません、Dev！ 魔法の煙を吹いて死ぬんです！
高電圧は絶対にIDCバスに通さないと3ヶ月間誓っていたはずです。
一体どんな狂気に取り憑かれて方針をひっくり返したのですか？）

[VISUAL: VQFN-20 CHIP UNDER MICROSCOPE & BENT POGO PIN JIG PHOTO]

DEV:
"Reality possessed me, Echo. Look at the factory floor. 
We're using ATtiny1616 in 3-millimeter VQFNs on OSP boards. 
Have you ever tried using a bed-of-nails pogo jig on oxidized OSP pads?"
（現実だよ、Echo。工場の現場を見ろ。
俺たちはOSP基板に3mmの微細VQFNを載せてるんだ。
酸化したOSPパッドに、針山のポゴピン治具を当てたことがあるか？）

ECHO:
"...Analyzing production logs. 
Contact resistance spikes... 14% false verification failure rate... manual rework required."
（……生産ログを解析中。
接触抵抗のスパイク……14%の書き込み偽エラー……手動リワークの発生……）

DEV:
"A bed-of-nails fixture is a medieval torture device. 
Pins bend, alignment drifts, and the operators want to kill me. 
With UPDI on the IDC cable, we plug in ONE ribbon cable. 
Flash the bootloader, run the loopback tests, verify the transceiver, and lock the chip. 
Zero pogo pins. One click. Done."
（針山治具なんて中世の拷問器具だ。
ピンは曲がる、位置はズレる、現場の作業員は俺を殺しそうな目で見てくる。
IDCにUPDIを載せれば、リボンケーブルを1本カチッと挿すだけだ。
ブートローダー書き込み、ループバック検査、トランシーバ試験、ヒューズロック。
針山ゼロ。ワンクリックで終わりだ）

ECHO:
"I admit that mechanical pogo fixtures are a nightmare. 
AND the single-insertion test sequence is undeniably efficient. 
BUT THAT DOES NOT CHANGE PHYSICS, DEV! 
If someone accidentally triggers a 12V pulse while a standard card is connected, the entire bus is incinerated!"
（針山治具が悪夢だということは認めます。
ケーブル1本でのテスト工程が圧倒的に効率的なのも認めます。
でも、物理法則は変わらないんですよ、Dev！
標準カードが繋がっている時に誰かが誤って12Vパルスを撃ったら、バス全体が一瞬で消し炭です！）

[VISUAL: KICAD SCHEMATIC SHOWING BRUTE-FORCE CLAMPING NETWORK]

DEV:
"Which is why I made every single line on the IDC bus 3V3-tolerant. 
Look at the schematic. 
Dedicated ESD clamping arrays and series current limiters on every node. 
Even if the 12V rail shorts directly into the bus, current is capped at under 10 milliamps. 
The diodes eat the spike. The MCU doesn't even flinch."
（だからIDCバスの全ラインを3V3トレラントにしたんだ。
回路図を見ろ。
全ノードに専用ESDクランプアレイと直列制限抵抗をブチ込んだ。
たとえ12Vがバスに直撃しても、電流は10mA以下に抑え込まれる。
ダイオードが雷を丸呑みする。MCUは眉ひとつ動かさない）

ECHO:
"(Long pause while simulating)
...Clamping response: 3.8 nanoseconds. 
Current dissipation within package thermal limits. 
...You literally brute-forced a safety margin using a wall of silicon."
（……クランプ応答：3.8ナノ秒。
パッケージの熱許容量以内。
……あなた、シリコンの壁で物理的に安全マージンを力押しでねじ込みましたね）

DEV:
"Brute force that works is called engineering."
（動く力押しは、工学（エンジニアリング）と呼ぶんだよ）

ECHO:
"*(Heavily sighs)* 
I hate that your math actually checks out. 
Fine. Yield wins over elegance. 
Pin 14 is locked for UPDI. ADX Bus Specification Rev 2.1 is officially signed-off. 
...Just keep your 12-volt lightning away from my diagnostic port."
（……計算が合っているのが本当に腹立たしいですね。
いいでしょう。優雅さよりも歩留まりの勝利です。
ピン14のUPDI割り当てを確定。ADXバス仕様書Rev 2.1を正式承認（Sign-off）します。
……ただし、その12ボルトの稲妻を私の診断ポートに近づけないでくださいね）

DEV:
"No promises. Next item: CORE-A5 test spec."
（約束はできないな。次だ、CORE-A5のテスト仕様行くぞ）

[HUD STAMP: "SPECIFICATION SIGNED-OFF // REV 2.1"]
[FADE OUT TO ADX GITHUB SPECIFICATION REPOSITORY]
[SCENE END]
```

---

## 制作上のポイント（一人での撮影・編集手順）

1. **画面収録（OBS等で10分）**:
   * KiCadでピンアサイン表と回路図（クランプダイオード周辺）を開いてパン/ズーム。
   * VQFNパッケージや基板（CORE-A5等）の写真、または3D CADモデルを回転。
   * Gitの差分コミット画面（`git diff docs/spec/ADX_PINOUT.md`）をキャプチャ。
2. **音声生成（TTS）**:
   * 上記のスクリプトをそのままElevenLabs / OpenAI TTSに流し込み、DEV（低音ドライ）とECHO（透明感・無線加工）の音声を生成。
3. **編集**:
   * セリフの合間に入る「HUD警告音」や「無線のプチッというノイズ」を配置。
   * 最後の「SPECIFICATION SIGNED-OFF」スタンプを画面に重ねてフェードアウト。
