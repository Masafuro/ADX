# Episode 02 Metadata — Title & Description
*Target Script:* [../episodes/ep02_cheap_devboard_trap.md](../episodes/ep02_cheap_devboard_trap.md)

---

## Title（タイトル）

### メイン案（Podcast / YouTube 共通）
```text
Ep. 02: Don't Build a Cheap MCU Board (Hard Mode With Zero Budget)
```

### サブ案（規模差のインパクト重視）
```text
Ep. 02: 100 Boards vs. Millions: Why Cheap Dev Boards Are Harder Than Industrial Hardware
```

---

## Description — Podcast 向け短縮版（実文字数 529文字 / CRLF換算 531文字）

```text
Want to launch a cheap, general-purpose MCU board? Honestly—don't do it. Competing at 100 boards a year against Raspberry Pi and Arduino's millions is a ten-thousand-to-one volume trap. Episode 02 unpacks why CORE-U died repeatedly, why we moved shelves to 3D printing, and how USB Type-C forces an ugly three-way trade-off with no clean exit.

Note: AI-narrated via OmniVoice. Dramatized fiction inspired by ADX dev logs; may include inaccuracies or retired specs. Not a datasheet—verified specs: https://github.com/Masafuro/ADX
```

### 日本語訳（内容確認用）
> 安くて汎用的なマイコンボードを作ってみたい？ 正直に言うと――やめた方がいい。年間数百万台を出荷する Raspberry Pi や Arduino に対して年 100 台で挑むのは、生産規模 1 万倍差の罠です。第 2 話では、なぜ CORE-U が何度も廃版になったのか、なぜ置き場所を 3D プリンター界隈へ移したのか、そして USB Type-C が突きつける「綺麗な出口のない三択」を紐解きます。
>
> 注記：OmniVoice による AI ナレーションでお送りします。ADX 開発ログに基づく脚色されたフィクションであり、不正確な点や旧仕様を含む場合があります。データシートではありません――検証済み仕様：https://github.com/Masafuro/ADX

---

## Description — YouTube / Show Notes 拡張版（文字数制限が緩い媒体向け）

```text
Thinking about launching a cheap, general-purpose microcontroller board to enter the hardware market? Honestly—don't do it. Building a cheap board is way harder than designing an expensive industrial controller.

In Episode 02, we unpack why our budget USB core (CORE-U) kept getting killed and archived—and what it actually takes to survive at small-batch scale:
- Millions vs. 100: Going head-to-head with Raspberry Pi Pico and Arduino means fighting a 10,000-to-1 volume gap
- Switching shelves: Leaving the electronics hobbyist aisle (r/arduino, r/embedded) for 3D printer makers (r/3Dprinting)
- Ruthless subtraction: All-SMD assembly, no buzzer, no Grove headers, and no physical reset button
- The USB Type-C three-door trap: Give up cost (ORing IC), give up 5V/2A 10W power (single port), or give up single-cable convenience (dual dedicated ports)—knowing miswiring still isn't magically solved

Cheap hardware isn't easy mode—it's hard mode with zero budget.

---
Note: Narration is AI-generated via OmniVoice. Episodes of Off the Breadboard are dramatized fiction inspired by real ADX development logs and may include simplified details, technical inaccuracies, or retired specs. Please don't treat this audio as a datasheet.

Current verified schematics & specs on GitHub:
https://github.com/Masafuro/ADX
```
