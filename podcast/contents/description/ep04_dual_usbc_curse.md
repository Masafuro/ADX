# Episode 04 Metadata — Title & Description
*Target Script:* [../episodes/ep04_dual_usbc_curse.md](../episodes/ep04_dual_usbc_curse.md)

---

## Target SEO Keywords
* `USB-C` / `Dual USB-C` / `USB Type-C`
* `ESP32` / `MCU Board` / `Microcontroller`
* `Hardware Design` / `PCB Cost` / `BOM`
* `Arduino UNO` / `Connector Design` / `Hardware UX`

---

## Title（タイトル）

### メイン案（Podcast / YouTube 共通・バランス最強・80文字）
```text
Ep. 04: Why Dual USB-C Ports Look Ugly on ESP32 & MCU Boards—And the 2-Cent Trap
```

### サブ案 1（USB-C 帝国・デザイン批判フック・76文字）
```text
Ep. 04: How USB-C Killed Good Hardware Design (Dual-Port ESP32 & MCU Boards)
```

### サブ案 2（BOM・原価検索重視・75文字）
```text
Ep. 04: Dual USB-C on MCU Boards, Hardware UX, and the 2-Cent PCB Cost Wall
```

---

## Description — Podcast 向け短縮版（実文字数 538文字 / CRLF換算 540文字）

```text
Why do dual USB-C ports on modern ESP32 and MCU boards look so ugly—when four USB-A ports on a PC look great? Episode 04 unpacks why identical power/data USB-C twins feel like exposed compromises, how a 2-cent separate-SKU cost wall killed our external programmer in one day, and how the USB-C empire wiped out the Arduino UNO's clear two-connector UX.

Note: AI-narrated via OmniVoice. Dramatized fiction inspired by ADX dev logs; may include inaccuracies or retired specs. Not a datasheet—verified specs: https://github.com/Masafuro/ADX
```

### 日本語訳（内容確認用）
> PCの背面にUSB-Aポートが4つ並んでいるのはカッコいいのに、最近のESP32やマイコンボードの「USB-C 2本並び」はなぜあんなにダサく見えるのか？ 第4話では、同じ銀色の穴が電源用・通信用に割れた瞬間に「設計の言い訳」が露出する理由、外付けプローブを1日で墓場送りにした「2セント差の別SKU税」、そしてArduino UNOの美しい異種コネクタ文化を絶滅させたUSB-C帝国の罠を紐解きます。
>
> 注記：OmniVoiceによるAIナレーションでお送りします。ADX開発ログに基づく脚色されたフィクションであり、不正確な点や旧仕様を含む場合があります。データシートではありません――検証済み仕様：https://github.com/Masafuro/ADX

---

## Description — YouTube / Show Notes 拡張版（文字数制限が緩い媒体向け）

```text
Look at the back of a PC with four USB-A ports lined up in a row—it looks clean and capable. Now look at a modern ESP32 or microcontroller board with two identical USB-C ports side by side—one secretly for UART/data, one for USB/power. Why does it instantly feel so ugly?

In Episode 04, we unpack a same-day hardware speedrun—designing and burying a 16-part external programmer probe before dinner—and what it revealed about modern hardware UX and BOM economics:
- Symmetrical slots vs. exposed excuses: Why PC USB-A rows signal capability, while dual identical USB-C jacks break USB-C's "one universal cable" promise on sight
- The 2-Cent Brick Wall (Separate-SKU Tax): Why a stripped-down 16-component programmer ($3.81 assembled) came out only two cents cheaper than a full MCU board ($3.83)—making a $5 indie dongle mathematically impossible
- The Valley of Doom: Why a $15 probe + $15 board ($30) gets crushed from below by the $20 Arduino UNO R4 Minima and from above by full-featured RP2040 probes
- Hostages of the USB-C Empire: Why the Arduino UNO's classic two-connector affordance (round DC barrel jack + square USB-B) is now a museum exhibit—forcing modern boards into awkward dual USB-C compromises

One universal connector to rule them all—and yet here we are squinting at tiny silkscreen between two identical holes, praying a corner-store cable actually has data wires inside.

---
Note: Narration is AI-generated via OmniVoice. Episodes of Off the Breadboard are dramatized fiction inspired by real ADX development logs and may include simplified details, technical inaccuracies, or retired specs. Please don't treat this audio as a datasheet.

Current verified schematics & specs on GitHub:
https://github.com/Masafuro/ADX
```
