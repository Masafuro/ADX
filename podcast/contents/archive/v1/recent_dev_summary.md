# 直近の開発作業サマリー（Gitログより）

本ドキュメントは、直近のコミット履歴（2026年10月中旬〜10月24日、コミット `9feab8c` 〜 `decf786`）から、プロジェクト「ADX」で行われた主要な開発・ドキュメント・ハードウェア作業を体系的にまとめたメモです。

---

## 1. 全体ハイライト（何が進んだのか）

直近の作業は大きく以下の4つの領域に集中しています：

1. **RPR4 Smart Probe（RP2040）の復活と「Web MIDI API」へのプロトコルピボット**
2. **Hackaday.io 向け広報・週報自動化パイプラインの構築と発信**
3. **CORE-A5（ATtiny1616）のハードウェア設計・ピン配置マトリクス・テスト仕様策定**
4. **ADX ファミリ命名規則およびドキュメント体系の大規模再編**

---

## 2. カテゴリ別詳細

### ① RPR4 Smart Probe の復活と Web MIDI プロトコル採用
* **RPR4のアーカイブからの復帰**:
  * RP2040ベースのスマートプローブ/モデム「RPR4」を再設計・復活（`hardware/RPR4_Smart_Probe/revival_of_RPR4.md`）。
  * ネットリストおよびコンセプトの再定義。
* **通信基盤の大方針転換（Web Serial/WebUSB → Web MIDI SysEx）**:
  * Android Chrome における Web Serial 無効化問題、Firefox 非対応、OSアップデートによるAPI突然死リスクを回避するため、40年以上の実績を持つオープン規格 **Web MIDI API (SysEx)** を採用することを正式決定（`memo/web_midi_protocol_study.md`）。
  * RPR4（RP2040）を「インテリジェント・モデム」と位置づけ、ブラウザ/ホストとのUSB-MIDI通信と、現場の RS-485（LN-485）/LIN BREAK 自律生成の責務を完全分離。
* **3フェーズの検証ロードマップ策定 (`dev_plan.md`)**:
  * **Phase 1**: USB-MIDI（TinyUSB）基盤の検証（SysEx双方向、PWAでの送受信・7-to-8パッキング）。
  * **Phase 2**: RP2040 PIO による高精度 LIN BREAK（13bit+ Dominant Low）+ Sync Byte 生成。
  * **Phase 3**: ADX（ATtiny1616）へのOTW（Over-The-Wire）スケッチ書き込みプロトコル実証。

---

### ② Hackaday.io 向け週報・広報パイプラインと情報発信
* **自動変換パイプラインの構築 (`docs/blog/scripts/build_hackaday.py`)**:
  * `raw_assets/` の画像を自動リサイズ・圧縮（JPG/PNG最適化）。
  * Markdown原稿から Hackaday.io 互換のインラインスタイル付きHTML（`docs/blog/dist/`）を自動生成するビルドシステムを構築。
  * マルチCDN対応および画像パスの自動URL置換。
* **開発ログ・振り返り記事の公開**:
  * ADX プロジェクトの半年間の歩みを振り返る記事（`docs/blog/posts/2026-10-22.md`, `2026-10-22_half_year_review.md`）。
  * プロジェクト概要・コンセプトの洗練（`hackadayio_project_v1.md`, `v2.md`, `v3.md`）。
  * ADXを「筐体ネイティブなモジュラー電子機器のオープンプラットフォーム（3-tier standard: Mechanical / Electrical / Application）」として再定義。

---

### ③ CORE-A5（ATtiny1616）ハードウェア設計・ピン配置の最適化
* **MCUマッピングとピン配置の整合性向上**:
  * ATtiny1616 を搭載する「CORE-A5」のピンアサイン（UPDI、UART、RS-485方向制御、ADC、BUS信号）を更新。
  * `docs/spec/ADX_PINOUT.md`, `docs/spec/mcu_mapping/attiny1616.md` および日英の各MCUマトリクスドキュメントを同期・改訂。
* **ネットリスト更新とテスト仕様書の作成**:
  * 回路ネットリスト `Netlist_PCB1_2026-10-17.enet` を更新。
  * `hardware/CORE-A5/rev1/test_specification.md` を作成し、電源投入前チェック、各電源レール電圧、クロック精度、UPDI/UARTループバック、LN-485トランシーバ波形試験などのテスト手順を標準化。
* **Arduino実装の基準値レポート策定**:
  * `memo/REPORT/ARDUINO_REFERENCE_VALUE_REPORT.md` により、Arduino環境におけるピン番号・レジスタ・タイミング基準値を明文化。

---

### ④ ドキュメント体系・命名規則のガバナンス強化
* **ADX Family 命名規則の制定 (`memo/ADX_Family_naming.md`)**:
  * ボード、モジュール、拡張カード、プローブ等の命名規約を整理し、将来のハードウェア拡張に対応。
* **ドキュメント構造改革 (`memo/REPORT/ADX_DOCUMENTATION_ARCHITECTURE_PROPOSAL.md`)**:
  * 仕様書（`spec/`）、多言語ガイド（`ja/`, `en/`）、ブログ（`blog/`）、ハードウェア詳細（`hardware/`）の責務分離を提案・推進。
* **ユーザー動線・トーン＆マナーの策定**:
  * 初めて訪れた開発者が迷わないためのポータル設計（`USER_JOURNEY_AND_ENTRY_PORTAL_PROPOSAL.md`）。
  * 過度な表現を排し技術的事実を重視する監査レポート（`OVERSTATED_EXPRESSIONS_AUDIT_REPORT.md`）およびArduino向けトーン＆マナー指針（`WORKING_ARDUINO_TONE_AND_MANNER.md`）。

---

## 3. コミットログ抜粋（時系列降順）

* `decf786` (2026-10-24): docs: add Hackaday.io project v3 blog post and update smart probe dev plan
* `45013c3` (2026-10-24): docs: add development plan and Web MIDI protocol study memo for RPR4 Smart Probe
* `12deb01` (2026-10-22): docs: add ADX family naming convention documentation
* `b1e3c0a` (2026-10-22): docs: add revival documentation for RPR4 hardware probe
* `aeefe6f` (2026-10-22): feat: add RPR4 Smart Probe hardware design files and concept documentation
* `22e384a` (2026-10-22): docs: publish ADX 6-month review blog post and project assets
* `c438790` (2026-10-22): feat: add multi-CDN support to Hackaday build script and include project v1/v2 documentation posts
* `763c258` (2026-10-22): update (Hackaday週報パイプライン初期導入・アセット配置)
* `3e96f60` (2026-10-17): docs: update ATtiny1616 pin mapping, form factor guidelines, and CORE-A5 hardware netlist and test specifications
* `9457221` (2026-10-17): update (ADX_PINOUT, attiny1616 netlist)
* `a717498` (2026-10-16): docs: add reference value report for Arduino implementation
* `72041b2` (2026-10-16): update (トーン＆マナー、誇大表現監査)
* `a07a40d` (2026-10-16): update (ポータル設計、README改訂)
* `efc2ec2` (2026-10-16): update (CORE-A / CORE-R アーカイブ化、ドキュメント構造提案)
* `9feab8c` (2026-10-16): update CORE-A5 (CORE-A5初期ネットリスト・仕様検討)

---

## 4. YouTube動画コンテンツ化への切り口（示唆）

直近の作業から、YouTubeで非常に強い関心を引ける「技術的フック」が多数見出せます：

1. **「なぜWeb Serialを捨ててWeb MIDIを採用したのか？」**
   * スマホ（Android Chrome）からハードウェアをWeb完結で叩くための技術選定ドラマ（40年前のMIDI規格がいかに頑健か）。
2. **「RP2040のPIOを使って現場用通信波形（LIN BREAK）を自律生成する」**
   * マイコンのユニークな機能（PIO）を使った実装デモ。
3. **「ケース（87x48mm）から逆算して基板を設計する理由（Enclosure-First Design）」**
   * ハードウェア開発のあるある失敗（基板を先に作ってケースに入らない）への実践的アプローチ。

