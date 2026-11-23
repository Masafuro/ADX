# Podcast Idea: WebSerialが死んでいるなら、1983年のMIDIで工場のRS-485を叩け

* **ステータス**: ストック（v4パイロット試作あり／v5本編 `episodes/` 未公開の最強飛び道具ネタ）
* **出典・関連資料**:
  * [podcast/contents/archive/v4/episodes/ep01_web_midi.md](../archive/v4/episodes/ep01_web_midi.md)
  * [podcast/contents/archive/v1/script_web_midi_en.md](../archive/v1/script_web_midi_en.md)
  * [memo/SMOKINGROOM/2026-11-19_rpr4_and_ln485_honest_talk.md](../../../memo/SMOKINGROOM/2026-11-19_rpr4_and_ln485_honest_talk.md)
  * [software/sandbox/probe_test_result3.md](../../../software/sandbox/probe_test_result3.md)

---

## 1. コア・フック（1行あらすじ）

工場のハシゴの上で Android スマホ1台から産業用 RS-485 機器へファームウェアを書き込むため、モダンな **WebSerial** を捨てて **1983年のシンセサイザー規格（Web MIDI SysEx）** を採用し、間に挟んだ **RP2040 の PIO** に「80年代のシンセ音楽」を「マイクロ秒精度の産業用 LIN BREAK 電圧パルス」へ翻訳させた痛快ハックの話。

---

## 2. v5 制作パイプライン用 4要素メモ

1. **The Taboo（DEVの禁忌）**:
   * 「産業用の現場プログラマから WebSerial を引っこ抜いて、MIDI に差し替えた。そう、1983年の電子楽器・シンセサイザーの規格だ。」
2. **The Sarcastic Attacks（ECHOの猛攻）**:
   * 「帯域の無駄遣いだ！ MIDI SysEx は8ビットのバイナリを7ビットに詰め直すから、スタート地点でいきなり通信速度の **12.5%** をドブに捨てることになるぞ！」
   * 「もっと致命的なのはタイミングだ！ うちの RS-485 バス（LN-485）はマイコンを起こすために **13ビット幅のドミナントLow（LIN BREAK）** が要るんだぞ。楽器用MIDIケーブルがマイクロ秒単位で産業シリアル線を引っ張れるわけないだろ！」
3. **The Metaphor Bite（刺さる比喩・逆転）**:
   * **ハシゴの上の文鎮スマホ**：「Google は Android Chrome で WebSerial を標準無効にした。Mozilla は Firefox への実装を拒否した。美しい W3C 規格を守った結果、あんたのスマホはハシゴの上で文鎮になる。帯域の12%を節約したんじゃない――**接続の100%を失ったんだ**。」
   * **80年代シンセソロの変装**：「モバイルブラウザの検問を突破するため、産業用ファームウェアに80年代シンセサイザー・ソロの衣装を着せ、プローブ内の RP2040 の独立した PIO ステートマシンに、その“音楽”を現場の電圧パルスへと翻訳させている。」
   * **Android の OS ジッター実測の泥沼**（`probe_test_result3.md` の実話も補強可能）：スマホの `bulkTransfer`＋ボーレートトリックでは OS ジッターに物理的に勝てず `0/3 TIMEOUT` の嵐だったからこそ、タイミング生成を RP2040 PIO 側へ丸投げする「インテリジェント・モデム」構成が必然になった。
4. **The Math Payoff（反論不能な数字・哲学）**:
   * Web MIDI は2015年から主要ブラウザに標準搭載・ドライバ不要（※**重要注意**：iOS Safari 単体は Web MIDI 非対応なので、誤って「iOS Safari 直結」と言わないこと！）。
   * ターゲット ATtiny1616 はたった 16KB Flash → 12.5%の帯域ペナルティは書き込みあたり **わずか約1.2秒** の差（実機ベンチでは SysEx 16KB を `517ms`・ロス `0.00%`、PIO の LIN BREAK `112.00 µs` 誤差 `-0.75%`・1,000連発 0エラー）。
   * **結びの哲学**：「デバイスマネージャに産業プローブが“楽器”として出るのは変か？ 間違いなく変だ。だがエンジニアリングはスペック表の美人コンテストじゃない――ハシゴの途中で確実に動くものを作ることだ。」

---

## 3. SEOキーワード・タイトル案

* **Target SEO Keywords**: `Web MIDI`, `WebSerial`, `RS-485`, `RP2040 PIO`, `Embedded`, `Android USB`
* **タイトル案**:
  * `Ep. XX: Why I Replaced WebSerial With a 1983 Music Standard (Web MIDI + RP2040 PIO)`
  * `Ep. XX: Hacking Industrial RS-485 From a Browser Using 1983 MIDI & RP2040 PIO`

---

## 4. 注意・ファクトチェックメモ

* 既存 v4 試作（`archive/v4/episodes/ep01_web_midi.md`）をベースに、**v5 ルール（製品名の `ADX RPR 4` 読み上げ表記、実測値、iOS Safari 誇張の完全排除、SEOタイトル＆600字以内概要欄）**へブラッシュアップするだけで、超ハイクオリティな1本が完成する。
* 筐体・BOM・寸法・USB-C といったハード物理系が続いたあとに投入すると、番組のレンジ（ソフト×プロトコル×シリコン）が一気に広がる。
