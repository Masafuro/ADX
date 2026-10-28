# Tactical Design Review #03: The 105°C USB-C Connector Dilemma
## Why Every Component Survived -40°C~+105°C Except the $0.15 USB Port

本ドキュメントは、番組設計書（`youtube_show_bible.md`）の **Mode A: Pre-Flight Review（設計審議型）** および「3大黄金ルール」に基づいて作成された動画台本です。

「基板上のIC・受動部品はすべて産業用（-40℃〜+105℃）で揃えたのに、安価なUSB Type-Cコネクタだけが耐熱定格（85℃上限）をクリアできない」という、ハードウェアエンジニアなら誰もが一度は発狂するリアルなジレンマを、緊迫した技術ディベートとシニカルなユーモアで描きます。

---

## 企画概要

* **タイトル (Title)**: *Our Board Survives 105°C... Except the $0.15 USB Connector*
* **サブタイトル (Subtitle)**: *ADX Tactical Design Review #03 — The Melted Plastic Paradox*
* **所要時間**: 約 3分30秒
* **登場人物**: 
  * **DEV**: 現場至上主義のマッド・プラグマティスト。「溶けたら自動防振ロックだ」と言い放つ。
  * **ECHO**: 正論と安全規格を背負った胃痛のツッコミAI。「虚偽記載で訴訟を起こされますよ！」とキレ散らかす。
* **テーマ**: -40℃〜+105℃耐熱の産業用BOMにおいて、USB 6Pコネクタの85℃定格限界をどう工学的に突破・正当化したか？
  * 産業用105℃コネクタの絶望的なコスト（1個$18）と納期（48週）
  * LCP樹脂の熱変形温度（260℃リフロー耐熱）と「挿抜定格」の工学的本質
  * 現場運用（Field Deployment）における「IDC給電 ＋ USB無通電・無挿抜」アーキテクチャ分離

---

## 演出・画面アセット指示

* **映像 (Visual)**:
  * 暗い画面、青と緑のHUDオーバーレイ。
  * 画面中央にCORE-AのBOM監査シート（全コンポーネントが緑色の「-40℃~+105℃」の中、J1のUSB-Cだけが赤く点滅）。
  * DigiKey/Mouserの検索結果（高額な軍用コネクタ）、USB Type-C 6P断面図、LCP樹脂の熱特性グラフ。
* **音響 (Audio)**:
  * 冷却ファンの静かな風切り音、無線のスケルチノイズ。
  * エラー検知時のシャープな警告ビープ音。

---

## 本編台本（英語スクリプト ＆ 日本語対訳）

```text
[SCENE START]
Visual: Dark terminal screen. 
A BOM Compliance Audit is scanning across the screen. 
MCU (ATtiny1616) ... -40°C to +125°C [PASS]
RS-485 Transceiver ... -40°C to +105°C [PASS]
LDO Regulator ... -40°C to +125°C [PASS]
Passives (0603 X7R) ... -55°C to +125°C [PASS]
Connector J1 (USB Type-C 6P) ... -25°C to +85°C [FAIL - CRITICAL NON-COMPLIANCE]
Audio: Warning tone. ECHO's link initializes with static.

ECHO:
"Dev. I ran the thermal compliance audit for the CORE-A production BOM. 
You stamped 'INDUSTRIAL SPEC: -40°C to +105°C' across the front panel. 
Yet, Connector J1—the USB Type-C debug port—is rated for a maximum operating temperature of EIGHTY-FIVE DEGREES. 
You are one thermal chamber test away from a catastrophic fraud lawsuit."
（Dev。CORE-A量産BOMの熱規格コンプライアンス監査を実行しました。
フロントパネルに『産業用仕様：-40℃〜+105℃』とデカデカと刻印しましたね。
ですが、コネクタJ1……USB Type-Cデバッグポートの最大動作温度定格は85℃です。
環境試験室で一発テストされたら、詐欺で訴訟を起こされますよ）

DEV:
"It's fine. Nobody plugs a USB cable into an engine bay at 105 degrees."
（問題ない。105℃のエンジンルームの中でUSBケーブル挿す奴なんていない）

ECHO:
"'Nobody plugs it in'?! Is that your formal defense in product liability court?! 
Thermal failure is not just about human operation! 
When an industrial control cabinet in the desert hits 105°C, the plastic housing softens! 
The contact springs relax, adjacent pins short together, and 24-volt rail power dumps into the MCU! 
Are you going to tell the fire marshal, 'Don't worry, nobody was holding the cable'?!"
（『挿す奴はいない』？！ 製造物責任訴訟でそれがあなたの正式な抗弁ですか？！
熱破壊は人間が操作するかどうかの問題ではありません！
砂漠の制御盤が105℃に達した瞬間、プラスチックハウジングが軟化するんです！
コンタクトのバネ圧が抜け、隣接ピン同士が短絡し、24Vラインの電力がMCUに直撃するんですよ！
火災調査官に『ご安心ください、誰もケーブルを握っていませんでした』とでも言う気ですか？！）

DEV:
"If the plastic melts and fuses the pins together, it becomes a permanent vibration-proof mechanical lock. Free feature."
（プラスチックが溶けてピン同士が融着すれば、二度と抜けない永久耐振ロックになる。無料の新機能だ）

ECHO:
"THAT IS CALLED AN ELECTRICAL FIRE, DEV! 
Why won't you just use an automotive-grade, 105-degree rated USB connector like a sane engineer?!"
（それは電気火災と呼ぶんです、Dev！！
なぜ正気なエンジニアのように、車載グレードの105℃定格USBコネクタを採用しないのですか？！）

[VISUAL: COMPONENT DISTRIBUTOR SEARCH QUERY: "USB TYPE-C 105C INDUSTRIAL"]

DEV:
"Because I looked at the distributor inventory, Echo. 
Look at the screen. 
Automotive-grade Amphenol 105°C USB-C receptacle: 
Unit price: 18 dollars and 40 cents. 
Minimum order quantity: 5,000 units. 
Lead time: 48 WEEKS."
（ディストリビューターの在庫を見たからだよ、Echo。
画面を見ろ。
車載グレードのAmphenol製105℃対応USB-Cコネクタ：
単価：18ドル40セント。
最小発注数量：5,000個。
納期：48週間だ）

ECHO:
"...Lead time: forty-eight weeks?! That is nearly an entire calendar year!"
（……納期48週？！ ほぼ丸1年じゃないですか！）

DEV:
"Our entire board BOM is under twelve dollars. 
You want me to triple the product cost and halt production until next autumn, just to satisfy a marketing bullet point?"
（基板全体のBOMコストは12ドル以下だ。
マーケティングの箇条書き1行を満たすためだけに、原価を3倍にして来年の秋まで生産を止めろと？）

ECHO:
"I admit 48 weeks of lead time is economic suicide. 
HOWEVER. Physics does not care about component lead times! 
How do you prevent terminal degradation at 105°C?!"
（48週の納期が経済的自殺だということは認めます。
しかし！ 物理法則は部品の納期なんて知ったことではありません！
105℃での端子劣化をどうやって防ぐつもりですか？！）

[VISUAL: DATASHEET CROSS-SECTION: LCP RESIN HEAT DEFLECTION TEMPERATURE]

DEV:
"By reading the actual materials science instead of the top-line rating. 
What material is the insulator of that 15-cent USB connector made of?"
（表面の定格じゃなく、本物の材料工学を読んだからだ。
その15セントのUSBコネクタのインシュレータ樹脂は何でできてる？）

ECHO:
"...Analyzing material composition. 
Polymer: LCP—Liquid Crystal Polymer. 
Heat Deflection Temperature: 260°C for lead-free reflow soldering."
（……材料組成を解析。
ポリマー：LCP（液晶ポリマー）。
鉛フリーリフローはんだ付け用熱変形温度：260℃……）

DEV:
"Exactly. The connector already survived a 260-degree reflow oven during assembly without melting. 
It's not going to puddle into soup at 105°C."
（その通りだ。コネクタは製造工程で260℃のリフロー炉を生き残って実装されてる。
たかだか105℃でスープみたいに溶けるわけがない）

ECHO:
"Surviving a reflow oven is static! 
The 85-degree rating is for continuous mating cycles under electrical load!"
（リフロー炉を生き残るのは無通電の静的状態です！
85℃の定格は、通電負荷状態での連続挿抜サイクルに対する保証ですよ！）

[VISUAL: SYSTEM ARCHITECTURE BLOCK DIAGRAM: IDC BUS vs USB PORT]

DEV:
"Which brings us to the operational architecture. 
The USB port is defined as 'Bench Commissioning & Diagnostic Only'. 
When the unit is deployed in the field, the USB cable is UNPLUGGED. 
Field power and communications run 100% through the 20-pin IDC bus—which uses phosphor bronze and 105-degree LCP headers. 
At 105°C, the USB port has zero volts, zero current, and zero mating force. 
Zero load means zero degradation."
（そこでシステムアーキテクチャの出番だ。
USBポートは『ベンチ初期設定＆診断専用』と定義した。
ユニットが現場に配備されたとき、USBケーブルは『抜線状態』が仕様だ。
現場の電源と通信は100%、20ピンIDCバスから供給される。こっちはリン青銅と105℃定格のヘッダだ。
105℃の過酷環境下では、USBポートには電圧ゼロ、電流ゼロ、挿抜応力ゼロ。
負荷ゼロなら、劣化要因もゼロだ）

ECHO:
"(Long pause. Relay clicks in the background)
...Thermal stress recalculation complete. 
Unmated, unpowered LCP structure maintains 1.5kV dielectric isolation up to 130°C. 
Architecture condition documented: 'USB Port: Service Use Only (-25°C to +85°C)'. 
...I hate that your technical loophole is legally and thermally airtight."
（……熱応力の再計算完了。
無通電・無挿抜状態のLCP構造は、130℃まで1.5kVの絶縁耐圧を維持。
アーキテクチャ条件を追記：『USBポート：サービス利用限定（動作温度 -25℃〜+85℃）』。
……あなたの脱法ハックが、法規的にも熱的にも完全に隙がないのが本当に腹立たしいですね）

DEV:
"Air-tight engineering beats an 18-dollar connector every single time. Sign it off."
（隙のない工学は、いつだって18ドルのコネクタに勝つんだよ。サインしろ）

ECHO:
"*(Resigned sigh)* 
BOM Revision 1.4 approved. Component purchase order cleared for assembly. 
...If I detect a single milliamp on that USB port during environmental testing, Dev, I am initiating fire suppression directly onto your keyboard."
（……BOMリビジョン1.4を承認。基板実装用の部品発注を許可します。
……もし環境試験中にそのUSBポートから1ミリアンペアでも電流を検知したら、Dev、あなたのキーボードに直接消火剤を噴射しますからね）

DEV:
"Understood. Moving to the next item: LIN bus oscillator drift."
（了解。次行くぞ、LINバスの発振器ドリフトだ）

[HUD STAMP: "BOM SPECIFICATION SIGNED-OFF // REV 1.4"]
[FADE OUT TO ADX OPEN HARDWARE REPOSITORY]
[SCENE END]
```

---

## 制作上のポイント（一人での撮影・編集手順）

1. **画面収録（OBS等で10分）**:
   * KiCadのPCBレイアウトでUSBコネクタ周辺（J1）を拡大。
   * DigiKeyやMouserで「USB-C 105C」と検索し、単価$18や「Lead time: 48 weeks」の文字を画面に映す。
   * LCP樹脂の物性表（リフロー260℃）をハイライト。
2. **音声生成（TTS）**:
   * DEV（低音ドライ・飄々とした職人）とECHO（透明感・皮肉混じりの無線AI）の音声をTTSで生成。
3. **編集**:
   * 冒頭の「BOM Compliance Audit」の警告音と、最後の「BOM SPECIFICATION SIGNED-OFF」のHUD演出を配置。

