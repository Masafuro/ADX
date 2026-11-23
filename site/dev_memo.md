<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ARLE.cc / ADX サイト開発メモ (`site/dev_memo.md`)

- **最終更新**: 2026-11-23
- **現在フェーズ**: **M1 完了 ＋ ビジュアル刷新（白 / 黒 / JIS CUD 黄）ローカル実装完了**
- **本番 URL**: `https://arle.cc`（Cloudflare Pages 自動ビルド）
- **注意**: 本ファイルは `site/` 直下に配置しているため Astro のビルド出力（`dist/`）には含まれません（開発者・AI エージェントの再開用メモ）。

---

## 1. 現在の状況（2026-11-23 時点の到達点）

### 完了済み
1. **Astro 5 ＋ Starlight 基盤の立ち上げ ＆ Cloudflare Pages 開通（M1）**
   - `astro@5.18.2` ＋ `@astrojs/starlight@0.37.7` を `site/` に導入。
   - Cloudflare Pages（`Root directory: site`, `Build command: npm run build`, `Output: dist`, **`NODE_VERSION = 22`**）で `https://arle.cc` の本番表示を確認済み（※Node 20 では依存パッケージ警告が出たため 22 を採用）。
   - 多言語ルーティング確定：英語デフォルト（`/`・`/adx/...`）＋日本語（`/ja/`・`/ja/adx/...`）。
   - 英語専用 SSOT ページの未翻訳フォールバック（`/ja/adx/standards/`）および Pagefind 日英ローカル全文検索の動作を確認済み。
2. **ビジュアル・トーン＆マナーの確定 ＆ `site/` 再構築（ローカル検証済み）**
   - 世界の主要 PLC メーカー 11 社＋Arduino Pro・CUD 規格を調査し、[**白ベース × 黒銘板 × JIS Z 9103 CUD 黄（`#F2E700`）**](../memo/ARLE_VISUAL_TONE_AND_MANNER.md) を正式方向性として策定。
   - `ThemeProvider.astro` / `ThemeSelect.astro` のオーバーライドにより、OS 設定に左右されない **Light（白ベース）既定＋ゼロ FOUC** を実装（右上の切替で Dark / Auto も利用可能）。
   - `custom.css`・`favicon.svg` を新規定へ刷新し、Spotify Podcast『**Off the Breadboard**』（`https://open.spotify.com/show/79cvRtpnEFaMkGB9mDjPpd`）の導線をヘッダー・トップ LP・ADX 概要ページへ組み込み済み。
   - `npm run build`（3.44s）および `astro preview` 全ルート `200 OK` を確認済み。

### 未完了・次回再開時の最初の一歩
- [ ] **新ビジュアル（白 / 黒 / JIS CUD 黄）の実ブラウザ目視確認**（`cd site && npm run dev` で PC・スマホ・Dark 切替時の余白・ヘッダー・リンク視認性をチェック）。
- [ ] 問題なければ `site/` および関連 `memo/` をコミット・`main` push して `https://arle.cc` 本番へ反映（※作業ツリーに別件の `memo/PLAN/archive/` 移動が含まれているため、コミット対象を確認すること）。
- [ ] Cloudflare Pages 側 **Settings → Build → Build watch paths（Include paths）** に `site/*`, `docs/*` が設定されているか確認（M2 の `docs/` 更新自動ビルド用）。

---

## 2. 確定済みの設計指針（次回以降もブレないための基準）

1. **ビルド運用**: ソースのみ Git 管理し、Cloudflare Pages 側で自動ビルドする（生成 HTML・Pagefind バイナリは Git にコミットしない）。
2. **URL プレフィックス**: ADX 関連は `/adx/...`（日本語 `/ja/adx/...`）に集約し、将来 ARLE.cc 配下に別シリーズが増えても衝突しない構造を維持。
3. **ビジュアル規範（`ARLE-GUIDELINE-VISUAL-001`）**:
   - **色と面積比（`82 : 15 : 3`）**: 純白 `#FFFFFF`（＋RAL 7035 灰 `#F4F5F7`）／漆黒 `#12151C`／改定 JIS Z 9103 CUD 黄 `#F2E700`。
   - **構図ハイブリッド**: 全 Docs は「① 黒銘板ヘッダ＋白ベース」、トップ LP のファーストビューのみ「④ 黒ヒーロー帯 → 純白セクション」。
   - **絶対禁止**: `#F2E700` を白地上の文字・細線に直接使わない（必ず `#12151C` とペア運用）。黄色面積を 3〜5% 以内に抑え、工事現場のトラ柄化を防ぐ。
   - **本文リンク**: 現行は「3 色完結（黒太字＋黄下線／ホバー淡黄マーカー）」。M2 で長文仕様書を取り込んだ際にリンク密集で見づらければ、JIS 濃青 `#005A9C` への切替を再評価する。
4. **SSOT 疎結合**: 厳密な英語仕様（`docs/spec/`）や公式ガイド（`docs/en/`, `docs/ja/`）は `site/` に手コピーせず、M2 で導入するビルド時同期スクリプトから取り込む。
5. **既存ツール保護**: GitHub Pages 上の `docs/index.html`（BREAK Explorer）・`docs/flasher/`（WebSerial Flasher）および `docs/blog/`（jsDelivr CDN）には一切干渉しない。

---

## 3. 今後の課題とロードマップ（M2〜M4）

### M2：公式規格の SSOT 自動同期 ＆ 筐体・回路ハブ整備（次の主要タスク）
- [ ] **`site/scripts/sync-docs.mjs` の実装**
  - `predev` / `prebuild` フックで `../docs/spec/`・`../docs/en/`・`../docs/ja/` から `site/src/content/docs/_synced/`（`.gitignore` 済）へ Starlight 用フロントマターを付与して同期する。
- [ ] **M2 で必ず対処すべき技術課題 2 点**
  1. **相対リンクの自動書き換え**: `docs/` 側の Markdown には `../spec/ADX_PINOUT.md` や `../../hardware/...` などの GitHub 相対リンクが含まれるため、サイト内ルート（`/adx/...`）または GitHub URL へ正しく変換するルールが必要。
  2. **LaTeX 数式表記の扱い**: `docs/spec/ADX_PINOUT.md` や `docs/en/8858_formfactor.md` 内に `$2 \times 2$`・`$V_{IL} \le 0.8\,\text{V}$` 等の TeX 数式が含まれている。Starlight 標準では数式が素の `$...$` で表示されるため、`remark-math` ＋ `rehype-katex` を導入するか、同期スクリプトで処理するかを判断する。
- [ ] **新規セクション公開**:
  - `/adx/enclosure/`（筐体設計者向けハブ ＋ `8858 Form Factor`）
  - `/adx/circuit/`（回路・CARD 設計者向けハブ ＋ `ADX 20-Pin Bus v4.2` / `MCU Mapping` / `Guidelines`）
  - `/adx/archive/`（旧 `8854` / `8748` の参照用隔離）

### M3：ボード図鑑 ＆ Arduino 風ビジュアル図面（SVG）
- [ ] **ボード図鑑 3 ページ公開**: `/adx/hardware/core-u/`、`/adx/hardware/reference-card/`、`/adx/hardware/prototype-card/`（1 ボード 1 ページ清書フォーマット）。
  - *検討課題*: `Prototype CARD v0`（現 `8854` 準拠）と `Reference CARD v0`（`8858` 準拠）の過渡期差異をステータスバッジでそのまま出すか、`Prototype CARD` の 8858 改訂と合わせるか。
- [ ] **共通 SVG 図面 2 点の作成（JIS Z 9103 / CUD v4 統一配色・日英共通）**:
  - `adx-bus-v4.2-pinout.svg`（20P IDC カラーピンアウト図）
  - `8858-mechanical-zones.svg`（筐体設計者向け寸法・M3 クリアランス・ガイド・光学マーカー図）
- [ ] **実基板到着後**: レンダリング画像を実機写真（RAL 7035 `#F4F5F7` 台座上）へ差し替え、ボード別ピンアウト SVG および STEP / DXF ダウンロードを追加。

### M4：ソフトウェア ＆ 作例（CORE-U 実機稼働後）
- [ ] `/adx/software/` 有効化：Arduino IDE 1.8.x ＋ `megaTinyCore` セットアップ、`ADX_<BOARD>.h`（`ADX_P01`–`P10`）コードリファレンス、WebSerial Flasher 導線。
- [ ] 完成作例（Desktop Andon 等）のプロジェクト記事追加。

---

## 4. 関連ドキュメント ＆ コマンド早見表

- **ビジュアル・トンマナ規定**: [`memo/ARLE_VISUAL_TONE_AND_MANNER.md`](../memo/ARLE_VISUAL_TONE_AND_MANNER.md)
- **サイト全体設計・マイルストーン**: [`memo/PLAN/ARLE_STARLIGHT_SITE_ARCHITECTURE_PLAN.md`](../memo/PLAN/ARLE_STARLIGHT_SITE_ARCHITECTURE_PLAN.md)
- **世界 PLC メーカー Web 調査レポート**: [`memo/REPORT/PLC_WEBSITE_TONE_AND_MANNER_RESEARCH_REPORT.md`](../memo/REPORT/PLC_WEBSITE_TONE_AND_MANNER_RESEARCH_REPORT.md)
- **執筆・用語トンマナ規定**: [`memo/ADX_TONE_AND_MANNER.md`](../memo/ADX_TONE_AND_MANNER.md)
- **運用ガイド**: [`site/README.md`](README.md)
- **Spotify Podcast（Off the Breadboard）**: https://open.spotify.com/show/79cvRtpnEFaMkGB9mDjPpd

```bash
# ローカル開発サーバー（http://localhost:4321）
cd site && npm run dev

# 本番相当の静的ビルド ＆ プレビュー
cd site && npm run build && npm run preview
```
