<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# 一次検証済み文献・ツール・標準リファレンス台帳（Verified Reference Ledger）

- **作成日**: 2026-11-28
- **方針**: 本台帳には、Web 一次検索により **タイトル・著者（または開発元）・年号・arXiv ID / DOI / 公開ドメイン・技術要点** を直接確認した文献・ツールのみを記載する。未確認・推測による引用は一切含めない。

---

## 1. 学術論文・サーベイ・ベンチマーク（査読会議・arXiv）

| Ref ID | タイトル | 著者・所属 | 年 / 会議・ID | 対応トラック | 確認済みリンク・識別子 |
| :---: | :--- | :--- | :--- | :---: | :--- |
| **[R01]** | *Surveying GenAI-based Automation in Printed Circuit Board Design and Test* | Sahana Srinivasan, Benjamin Turnbull, Hammond Pearce | 2026 / arXiv | 全体 | [arXiv:2606.17074](https://arxiv.org/abs/2606.17074) |
| **[R02]** | *A Survey of Research in Large Language Models for Electronic Design Automation* | Pan et al. | 2025–2026 / ACM TODAES / arXiv | 全体 | ACM TODAES / arXiv |
| **[R03]** | *PCBSchemaGen: Reward-Guided LLM Code Synthesis for Printed Circuit Boards (PCB) Schematic Design with Structured Verification* | Huanghaohe Zou, Peng Han, Emad Nazerian, Mafu Zhang, Zhicheng Guo, Alex Q. Huang (UT Austin) | 2026 / arXiv | Track 1 | [arXiv:2602.00510](https://arxiv.org/abs/2602.00510) |
| **[R04]** | *HWE-Bench: Can Language Models Perform Board-level Schematic Designs?* | Weibo Qiu, Yinhao Xiao, Runyu Pan | 2026 / arXiv | Track 1 | [arXiv:2603.18102](https://arxiv.org/abs/2603.18102) |
| **[R05]** | *PCB-QA: Evaluating LLMs over the First Printed Circuit Board Design Question-Answer Dataset* | Sahana Srinivasan, Benjamin Tan, Benjamin Turnbull, Hammond Pearce | 2026 / arXiv | Track 1 | [arXiv:2606.23704](https://arxiv.org/abs/2606.23704) |
| **[R06]** | *CircuitLM: A Multi-Agent LLM-Aided Design Framework for Generating Circuit Schematics from Natural Language Prompts* | Khandakar Shakib Al Hasan, Syed Rifat Raiyan, Hasin Mahtab Alvee, Wahid Sadik | 2026 / arXiv | Track 1 | [arXiv:2601.04505](https://arxiv.org/abs/2601.04505) |
| **[R07]** | *AnalogCoder: Analog Circuit Design via Training-Free Code Generation* | Yao Lai, Sungyoung Lee, Guojin Chen, Souradip Poddar, Mengkang Hu, David Z. Pan, Ping Luo | 2024 / arXiv | Track 1 | [arXiv:2405.14918](https://arxiv.org/abs/2405.14918) |
| **[R08]** | *Schemato — An LLM for Netlist-to-Schematic Conversion* | Ryoga Matsuo, Stefan Uhlich, Arun Venkitaraman, Andrea Bonetti, Chia-Yu Hsieh, Ali Momeni, Lukas Mauch, Augusto Capone, Eisaku Ohbuchi, Lorenzo Servadei | 2024 / arXiv | Track 1, 2 | [arXiv:2411.13899](https://arxiv.org/abs/2411.13899) |
| **[R09]** | *MuaLLM: A Multimodal Large Language Model Agent for Circuit Design Assistance with Hybrid Contextual Retrieval-Augmented Generation* | Pravallika Abbineni, Saoud Aldowaish, Colin Liechty, Soroosh Noorzad, Ali Ghazizadeh, Morteza Fayazi (Univ. of Utah) | 2025 / arXiv | Track 1 | [arXiv:2508.08137](https://arxiv.org/abs/2508.08137) |
| **[R10]** | *PCB-Bench: Benchmarking LLMs for Printed Circuit Board Placement and Routing* | Jindong Li, Lianrong Chen, Bin Yang, Jiadong Zhu, Ying Wang, Yuzhe Ma, Menglin Yang | 2026 / ICLR 2026 | Track 1, 3 | OpenReview: `Q5QLu7XTWx` |
| **[R11]** | *SchGen: PCB Schematic Generation with Semantic-Grounded Code Representations* | Qinpei Luo, Ruichun Ma, Xinyu Zhang, Lili Qiu | 2026 / arXiv | Track 2 | [arXiv:2605.30345](https://arxiv.org/abs/2605.30345) |
| **[R12]** | *PCBWorld: A Benchmark Environment for Engine-Grounded PCB Design Automation* | Hyungseok Song et al. (LG AI Research) | 2026 / arXiv | Track 2, 4 | [arXiv:2607.05915](https://arxiv.org/abs/2607.05915) |
| **[R13]** | *Cypress*（GPU 並列解析的 PCB 配置フレームワーク） | Niansong Zhang, Anthony Agnesina, Noor Shbat, Yuval Leader, Zhiru Zhang, Haoxing Ren (Cornell Univ. & NVIDIA) | 2025 / ISPD 2025 (**Best Paper Award**) | Track 3 | ISPD 2025 Proceedings |
| **[R14]** | *PCBAgent: An Agent-based Framework for High-Density Printed Circuit Board Placement* | Lin Chen et al. | 2025 / ASP-DAC 2025 | Track 3 | DOI: `10.1145/3658617.3697736` |
| **[R15]** | *Learning Circuit Placement Techniques Through Reinforcement Learning with Adaptive Rewards* (`RL_PCB`) | Luke Vassallo, Josef Bajada | 2024 / DATE 2024 | Track 3 | GitHub: `LukeVassallo/RL_PCB` |
| **[R16]** | *Net Separation-Oriented Printed Circuit Board Placement via Margin Maximization* (`NS-Place`) | Cheng et al. | 2022–2023 / arXiv | Track 3 | [arXiv:2210.14259](https://arxiv.org/abs/2210.14259) |
| **[R17]** | *Rubber-band / Topological Routing 基礎理論*（Leiserson & Maley / Dai, Dayan & Staepelaere `SURF`） | C. E. Leiserson, F. M. Maley / W. W.-M. Dai et al. (UC Santa Cruz) | 古典基礎文献 | Track 4 | UC Santa Cruz / JITX 技術解説参照 |

---

## 2. OSS・商用 EDA ツール・プロトコル仕様

| Ref ID | ツール / 仕様 | カテゴリ | 対応トラック | 検証済み要点 |
| :---: | :--- | :--- | :---: | :--- |
| **[T01]** | **EasyEDA Pro Extension API / MCP** | ネイティブ EDA（ADX 基盤） | Track 2, 4 | `PCB_ManufactureData.getDsnFile()`, `PCB_Document.startAutoRouting()`, `importAutoRouteSesFile()`, `locked` 保護、公式/コミュニティ FreeRouting 拡張あり。 |
| **[T02]** | **Specctra DSN / SES Format** | 外部ルーター標準交換形式 | Track 2, 4 | `(type fix)` / `(type protect)` によるロック配線保護に対応。パス表現は直線ポリライン `(path layer width x1 y1 ...)` 基準。 |
| **[T03]** | **FreeRouting** (`freerouting.app` / `freerouting.org`) | OSS オートルーター | Track 4 | ヘッドレス CLI（`-de`, `-do`, `-ap`, `-pp`, `-mt`, `-oit`, `-us`, `-inc`）およびローカル WebSocket API（ポート `37864`）対応。EasyEDA Pro と DSN/SES 連携可。 |
| **[T04]** | **FastRoute** | OSS オートルーター（Rust） | Track 4 | FreeRouting の Rust 移植版。JVM 不要・マルチスレッド対応の DSN/SES 互換代替候補。 |
| **[T05]** | **OrthoRoute** | OSS GPU ルーター（KiCad） | Track 4 | GPU 並列 Manhattan PathFinder。巨大多層バックプレーン特化で、小〜中規模基板では FreeRouting より不利。 |
| **[T06]** | **TopoR** (Eremex) | 商用トポロジカルルーター | Track 4 | ゴム紐トポロジカル・任意角配線の商用基準。高密度・層数削減に強いが有料クローズド。 |
| **[T07]** | **`tscircuit`** | OSS Code-as-CAD (TS/React) | Track 2 | `<group>` 階層レイアウトと **`manual-edits.json`** による増分編集・上書き保護層が特徴。 |
| **[T08]** | **`atopile`** | OSS Code-as-CAD (`.ato`) | Track 1, 2 | モジュール階層と電気アサーション（IPC-2221 電圧間隔・定格照合）を備え、KiCad へコンパイル。 |
| **[T09]** | **`JITX`** | 商用 Code-as-CAD (Python) | Track 1, 2, 4 | Python による幾何配置・電気チェック・アンカーロックを提供。トポロジカル配線の実装限界に関する技術解説も公開。 |
| **[T10]** | **Quilter / DeepPCB / Flux / Cadence Allegro X AI** | 商用 AI-EDA | 全体, Track 4 | Quilter＝物理駆動で高密度デジタル向けだが高価・ブラックボックス。DeepPCB＝RL 配線だがアナログ・ビア品質に現場批判あり。 |
| **[T11]** | **Siemens HyperLynx DRC** | 商用 EMC/SI ルールチェッカー | Track 5 | 3D 幾何エンジン＋90+ ルールでリターンパス分断跨ぎ・ビア欠如・プレーン穿孔を高速検知。ネットクラス連動の説明可能レビューを実践。 |

---

## 3. SI/PI・EMC・DFM 技術標準および一次知見

| Ref ID | 出典 / 権威 | テーマ | 対応トラック | 検証済み要点 |
| :---: | :--- | :--- | :---: | :--- |
| **[S01]** | Dr. Howard Johnson / Dr. Eric Bogatin | 90° vs 45° vs 曲線配線、リターンパス物理 | Track 4, 5 | 90° 角の過剰容量 `~0.01–0.02 pF`（`~0.5 ps`）は一般デジタル帯域で電気的無害。真の SI/EMI 破綻要因はリターンパス分断・基準プレーン喪失・リターンビア欠如。 |
| **[S02]** | Rick Hartley | 2 層 vs 4 層スタックアップ、層遷移リターンビア | Track 5 | 信号ビア層遷移時は `1〜2` 線幅以内に GND リターンビアを近接配置。4 層（L2 連続 GND）で電磁界を局所封じ込め。 |
| **[S03]** | JLCPCB DFM ガイドライン / 化学エッチング・ピール強度知見 | 鋭角 `<90°` 酸トラップ、45° 推奨、QFN サーマルビア | Track 4, 5 | 工業 PCB は化学湿式エッチング（CNC 切削ではない）。鋭角 `<90°` は酸トラップ＆熱応力剥離の主因のため厳禁・45° 推奨。QFN サーマルビアは `0.20〜0.25 mm` 径で吸い込み防止。 |
| **[S04]** | IPC-2221 / IPC-7351B | 電圧ベース絶縁間隔・フットプリント Courtyard 標準 | Track 1, 課題 3 | ネット電圧に応じた沿面／空間距離、および部品本体＋実装・リワーク余白の Courtyard 基準。 |
