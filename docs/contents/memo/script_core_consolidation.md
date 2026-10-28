# The Graveyard of Prototypes: How 6 Failed Boards Birthed CORE-A
*ADX Tactical Design Review #04 — The Art of Subtraction*

---

### [ECHO]
> **EN:** Dev. I just completed a directory scan of your hardware repository. In the last six months, you spun CORE-V0, CORE-D, CORE-S-P, and CORE-R. Your lab bench is a monument of blue JLCPCB delivery boxes, and your Git history looks like an unholy graveyard of abandoned schematics.
> 
> **JA:** Dev。ハードウェアリポジトリの走査を完了しました。過去6ヶ月間で、あなた CORE-V0、CORE-D、CORE-S-P、そしてCORE-Rを発注していますね。作業台には青いJLCPCBの段ボール箱がピラミッドのように積み上がり、Gitログは放棄された回路図の不吉な霊園と化しています。

---

### [DEV]
> **EN:** They weren't abandoned, Echo. They were sacrifices. You cannot forge a flagship without a mountain of dead silicon.
> 
> **JA:** 放棄じゃない、Echo。生贄（いけにえ）だ。死んだシリコンの山がなければ、フラッグシップは鍛え上がらない。

---

### [ECHO]
> **EN:** 'Sacrifices'?! You designed CORE-S-P specifically to host a dedicated secondary microcontroller as a Baseboard Management Controller. You waited two weeks for manufacturing, populated the components, and then threw it straight into the scrap bin four days later because you decided 'secondary chips are inelegant'!
> 
> **JA:** 『生贄』ですって？！ あなた、ベースボード管理コントローラとして専用のセカンダリMCUを載せるためだけにCORE-S-Pを設計し、2週間待って製造させ、部品を手付け実装した挙句、4日後に『予備チップは美しくない』とか言ってゴミ箱に直行させましたよね！

---

### [DEV]
> **EN:** Because it was inelegant. A secondary BMC adds 80 cents to the BOM, occupies valuable surface area, and introduces another firmware attack surface. Why add silicon when you can use the internal hardware gates already sitting inside the ATtiny?
> 
> **JA:** 実際、美しくなかったからな。予備のBMCチップはBOMを80セント跳ね上げ、貴重な基板面積を占領し、ファームウェアの脆弱性ポイントを増やす。ATtinyの内部に最初から眠っているハードウェアゲートを使えば済む話なのに、なぜ余計なシリコンを載せるんだ？

---

### [ECHO]
> **EN:** You mean your Configurable Custom Logic hack?
> 
> **JA:** あなたの内蔵CCL（プログラマブル・ロジック）ハックのことですか？

---

### [DEV]
> **EN:** Exactly. The ATtiny1616 has internal hardware logic blocks. We configured the CCL to monitor the RS-485 line asynchronously. When it detects the calibrated break pulse, it asserts an autonomous hardware reset directly to the core—even if the CPU is locked in an infinite hard fault. Zero extra chips. Zero added BOM cost.
> 
> **JA:** その通りだ。ATtiny1616には内部ハードウェアロジックブロック（CCL）がある。RS-485ラインを非同期監視するようにCCLを配線した。調整されたブレークパルスを検知した瞬間、CPUがハードフォールトで無限ループ死していようが、コアへ直結で自律リセットを叩き込む。追加チップ、ゼロ。追加BOMコスト、ゼロだ。

---

### [ECHO]
> **EN:** So you completely bypassed the operating CPU and wired an invisible physical tripwire straight into the microcontroller's brain?! You are an absolute barbarian, Dev. BUT what about multi-drop flashing?! On CORE-S-P, your BMC handled individual node isolation! How do you target Board Number Three without burning Board Number One?!
> 
> **JA:** つまり動作中のCPUを完全に無視して、マイコンの脳みそに直結する透明なトラップワイヤーを仕掛けたと？！ あなたは正真正銘の野蛮人ですね、Dev。ですが、マルチドロップ接続での書き込みはどうするのです？！ CORE-S-PではBMCが個別ノードの切り離しを担っていました！ 1番の基板を巻き込まずに、3番の基板だけにどうやってファームを焼く気ですか？！

---

### [DEV]
> **EN:** We reset the whole bus simultaneously.
> 
> **JA:** バス上の全員を一斉に道連れでリセットする。

---

### [ECHO]
> **EN:** ...Excuse me?! You crash the entire industrial bus?!
> 
> **JA:** ……はい？！ 産業用バス全体をクラッシュさせる気ですか？！

---

### [DEV]
> **EN:** Every node enters bootloader mode together. Then, the host uploader queries the factory SIGROW—the unique silicon hardware signature burned into each ATtiny at the fab. Only the node matching the target signature accepts the flash payload. All other nodes time out in 200 milliseconds and quietly resume their tasks.
> 
> **JA:** 全ノードが一斉にブートローダーモードに入る。そこでホストツールがファクトリー出荷時にチップへ焼き込まれた個体固有シグネチャ『SIGROW』を問い合わせる。一致したターゲットだけがファームウェアを焼き、それ以外の全ノードは200ミリ秒でタイムアウトして静かに業務へ復帰する。

---

### [ECHO]
> **EN:** *(Long silence as simulation models run)* ...You basically staged a simultaneous hostage situation on the entire bus, checked their silicon DNA to pick the single target, and let the innocent nodes go back to sleep?! ...Simultaneous whole-bus reset eliminates individual power switches. SIGROW matching provides cryptographic hardware-level targeting. Combined with the native 3.3-volt bus conversion... all six experimental branches collapse into a single unified architecture.
> 
> **JA:** （……シミュレーションが走る長い沈黙）……つまり、バス全体を一斉に人質に取って、シリコンのDNA鑑定でターゲットだけを1人指名し、無関係なノードは二度寝させたってことですか？！ ……全バス一斉リセットにより、ノードごとの物理電源スイッチ回路が全廃。工場固有SIGROWマッチングにより、暗号レベルのハードウェア識別を実現。さらにネイティブ3.3Vバスへの統一により……6つに分岐していた実験基板が、すべて単一のアーキテクチャに集約される……

---

### [DEV]
> **EN:** The art of subtraction, Echo. We buried CORE-D, CORE-S-P, and CORE-R. In their place stands CORE-A: 87 by 48 millimeters, native 3.3-volt logic, autonomous CCL recovery, and industrial -40°C to +105°C survivability. The flagship is complete.
> 
> **JA:** 減算の美学だよ、Echo。俺たちはCORE-D、CORE-S-P、CORE-Rを土に埋めた。その墓標の上に立つのがCORE-Aだ。87×48ミリ、ネイティブ3.3Vロジック、自律CCLリカバリ、そして-40℃〜+105℃の極限耐熱。フラッグシップの完成だ。

---

### [ECHO]
> **EN:** *(Heavily sighs)* I hate that your trail of destroyed silicon actually resulted in an engineering masterpiece. Repository cleaned. Legacy prototype branches archived to the tomb. CORE-A designated as the sole official ADX Flagship MCU reference.
> 
> **JA:** （……重いため息）破壊されたシリコンの山が、結果的に工学の傑作を生み出したのが本当に腹立たしいですね。リポジトリの清掃完了。旧世代のプロトタイプブランチをすべて墓場（アーカイブ）へ移送。CORE-Aを、ADX公式唯一のフラッグシップMCUリファレンスとして正式指定（Sign-off）します。

---

### [DEV]
> **EN:** Send the Gerbers to the factory.
> 
> **JA:** 工場にガーバーデータを送れ。
