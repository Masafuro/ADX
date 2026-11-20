# Episode 03 Metadata — Title & Description
*Target Script:* [../episodes/ep03_one_millimeter.md](../episodes/ep03_one_millimeter.md)

---

## Target SEO Keywords
* `PCB Design` / `PCB Connector` / `PCB Panelization`
* `Hardware Size` / `DFM` (Design for Manufacturability)
* `Sony Walkman` / `Sony Design Rule`
* `SMD` / `V-Cut` / `Enclosure Design`

---

## Title（タイトル）

### メイン案（Podcast / YouTube 共通・バランス最強・70文字）
```text
Ep. 03: Why 1mm Broke Our PCB—And Sony's Walkman Rule on Hardware Size
```

### サブ案 1（Sony Walkman 先行フック・85文字）
```text
Ep. 03: Sony's Walkman Rule on PCB Size—Why Moving One Connector 1mm Broke Everything
```

### サブ案 2（DFM・設計検索重視・75文字）
```text
Ep. 03: Moving a PCB Connector 1mm—Sony's Design Rule, DFM & Enclosure Math
```

---

## Description — Podcast 向け短縮版（実文字数 551文字 / CRLF換算 553文字）

```text
Moving one PCB connector by 1mm—the thickness of a credit card—broke our legacy boards, CAD enclosures, and sixteen specs. Why did that single millimeter unlock SMD assembly while echoing Sony's Walkman engineering rule on hardware size? Episode 03 unpacks CAD's infinite-freedom curse, PCB panelization, DFM trade-offs, and why necessity must pick your dimensions.

Note: AI-narrated via OmniVoice. Dramatized fiction inspired by ADX dev logs; may include inaccuracies or retired specs. Not a datasheet—verified specs: https://github.com/Masafuro/ADX
```

### 日本語訳（内容確認用）
> 基板上のコネクタをたった1mm――クレジットカード1枚分の厚み――動かしただけで、過去のボード、筐体CAD、そして16本もの仕様書が全部壊れた。なぜその1mmがSMD実装を解禁し、初代ソニー・ウォークマン開発チームの「寸法の鉄則」と響き合ったのか？ 第3話では、CADが与える「無限の自由」の呪い、PCBパネル面付けとDFMの攻防、そして「必然性に寸法を決めさせる」現実を紐解きます。
>
> 注記：OmniVoiceによるAIナレーションでお送りします。ADX開発ログに基づく脚色されたフィクションであり、不正確な点や旧仕様を含む場合があります。データシートではありません――検証済み仕様：https://github.com/Masafuro/ADX

---

## Description — YouTube / Show Notes 拡張版（文字数制限が緩い媒体向け）

```text
We moved a single 20-pin IDC connector on our PCB by one millimeter—from Y=48 mm down to Y=47 mm—and invalidated legacy boards, CAD enclosures, silkscreen coordinates, and sixteen specification documents.

Sounds like petty obsession? In hardware design, one millimeter separates two different worlds:
- The V-cut vs. SMD fillet collision: Why Y=48 mm put automated surface-mount solder joints directly in the path of the factory V-cut blade, and how 1 mm unlocked full-SMD compatibility
- The Ruler Thought Experiment: Why CAD's "infinite freedom" is a curse—how adding 5 mm kills toolbox fitment, doubles postal shipping, and wrecks PCB panelization (the woodworker's cut list)
- Cornered from all four sides: How 100x100 mm DIN-rail cabinets and a 66 mm front I/O edge (Ethernet RJ45, spring-clamp terminals, D-Sub 9) squeezed our board into the 88x54 mm form factor and the 1 mm trench at Y=47 mm
- Sony's Walkman Engineering Mantra (Rule #3): "Never decide size or cost by what's possible. Decide them by necessity."

Don't pick a dimension because you can—let necessity corner you into it.

---
Note: Narration is AI-generated via OmniVoice. Episodes of Off the Breadboard are dramatized fiction inspired by real ADX development logs and may include simplified details, technical inaccuracies, or retired specs. Please don't treat this audio as a datasheet.

Current verified schematics & specs on GitHub:
https://github.com/Masafuro/ADX
```
