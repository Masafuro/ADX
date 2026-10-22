<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Blog & Hackaday.io Weekly DevLog リファレンス

本ディレクトリ (`docs/blog/`) は、ADXプロジェクトの **Hackaday.io 向け Weekly 開発ログ** の執筆・画像最適化・HTMLビルドを自己完結して行うための作業区画です。  
ハードウェア・ファームウェアの本筋コードを散らかさず、最小限の手間でリッチな開発ログを生成できるように設計されています。

---

## 1. ディレクトリ構造

```text
docs/blog/
├── README.md               # [本ドキュメント] 投稿開発リファレンス
├── TONE_AND_MANNER.md      # [執筆指針] Hackaday.io 向けトンマナ・ライティングガイド
├── raw_assets/             # [作業用] スマホ写真やスクショなど未加工の生画像を配置
├── images/                 # [公開用] スクリプトにより最適化された画像 (GitHub Pagesで配信)
├── posts/                  # [原稿] 週報Markdown原稿 (YYYY-MM-DD.md)
├── scripts/                # [ツール] Hackaday特化型ビルドスクリプト (build_hackaday.py)
└── dist/                   # [成果物] Hackaday.io 貼り付け用HTMLスニペット (YYYY-MM-DD.html)
```

---

## 2. 記事作成ワークフロー (4ステップ)

開発者の実作業は **約3分** で完了します。

```text
[1. 写真配置]        スマホ写真等を docs/blog/raw_assets/ へ保存
       ↓
[2. 原稿執筆]        docs/blog/posts/YYYY-MM-DD.md を作成
       ↓
[3. ビルド実行]      python docs/blog/scripts/build_hackaday.py
       ↓
[4. Git & 投稿]      git commit & push 後、dist/YYYY-MM-DD.html を Hackaday に貼付
```

### Step 1: 生画像を配置 (`raw_assets/`)
撮影した写真や波形キャプチャを `docs/blog/raw_assets/` に配置します。
- 命名規則: `YYYY-MM-DD-<任意の名前>.<拡張子>`
- 例: `2026-10-22-hero.png`, `2026-10-22-scope.png`

### Step 2: Markdown原稿の執筆 (`posts/`)
`docs/blog/posts/YYYY-MM-DD.md` を作成します（[フォーマット仕様](#3-原稿フォーマット仕様-markdown) 参照）。

### Step 3: ビルドスクリプトを実行
```bash
# 全ての新規画像最適化 ＆ 全記事のHTML生成
python docs/blog/scripts/build_hackaday.py

# 特定日付のみビルド
python docs/blog/scripts/build_hackaday.py --date 2026-10-22
```

### Step 4: GitHub Push と Hackaday.io 公開
1. 変更をコミットして GitHub にプッシュします（GitHub Pages 経由で画像 URL が有効化されます）。
   ```bash
   git add docs/blog/
   git commit -m "docs(blog): add weekly devlog 2026-10-22"
   git push origin main
   ```
2. Hackaday.io のプロジェクトページを開き、**[New Project Log]** を作成します。
3. エディタのツールバーで **HTML モード (`<>` アイコン)** に切り替えます。
4. `docs/blog/dist/YYYY-MM-DD.html` の中身を全選択して貼り付け、タイトルを設定して **[Publish]** します。

---

## 3. 原稿フォーマット仕様 (Markdown)

原稿は `docs/blog/posts/YYYY-MM-DD.md` に YAML Frontmatter 形式でメタデータを記述します。  
※ コミュニティに響く文章表現やストーリーテリングのルールについては、必ず **[TONE_AND_MANNER.md](./TONE_AND_MANNER.md)** を参照してください。

### テンプレート
```markdown
---
title: "ADX Weekly #01: ATTiny1616 Pinout Matrix & WebSerial Tuning"
date: "2026-10-22"
hero_image: "2026-10-22-hero.png"
summary: "Tackling the tight pinout matrix on ATTiny1616 and perfecting serial break timings for browser-based diagnostics."
tags: ["hardware", "firmware", "webserial", "attiny1616"]
---

## The Challenge: When 20 Pins Feel Like 5
今週直面したハードウェア/ファームウェアの課題を簡潔に書きます。

## How We Solved It: Hardware Slew-Rate + Software Magic
どう解決したのか、試行錯誤や測定データを書きます。

![Oscilloscope Waveform](2026-10-22-scope.png "ATTiny1616 UPDI break signal timing")

> **Key Takeaway:** 学びや設計のコツをハイライトします。

```c
// 重要なコードスニペット
PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
```

## Weekly Progress Summary
- **Hardware:** 基板設計やアートワークの進捗
- **Firmware:** レジスタ最適化や通信処理の実装
- **Tools:** WebSerial Explorer 等のアップデート

## Next Steps & Community Thoughts
来週の予定や、コミュニティ（Hackaday閲覧者）への質問・意見募集を書きます。
```

### Frontmatter パラメータ一覧
| フィールド | 必須 | 説明 | 例 |
| :--- | :---: | :--- | :--- |
| `title` | ○ | 記事タイトル（HackadayのLog Titleに利用） | `"ADX Weekly #01: ..."` |
| `date` | ○ | 発行日（`YYYY-MM-DD`） | `"2026-10-22"` |
| `hero_image` | 任意 | トップに大判表示するアイキャッチ画像名 | `"2026-10-22-hero.png"` |
| `summary` | 任意 | 記事冒頭に太字リード文として配置される要約 | `"Tackling the tight ..."` |
| `tags` | 任意 | タグ一覧 | `["hardware", "firmware"]` |

---

## 4. 画像仕様と自動最適化ルール

`build_hackaday.py` は、Hackaday.io のレイアウト崩れを防ぐために画像を自動処理します。

1. **Hero 画像の自動 16:9 クロップ**:
   - ファイル名に `hero` が含まれる画像（例: `YYYY-MM-DD-hero.png`）または `hero_image` 指定画像は、中央を基準に **16:9** に自動トリミングされます。
2. **最大幅リサイズ ＆ EXIF自動回転**:
   - スマホ撮影による縦横向き情報（EXIF Orientation）を正位置に自動補正。
   - 横幅が 1200px を超える場合は **横幅 1200px** に自動リサイズ（アスペクト比維持）。
3. **ファイル形式と圧縮**:
   - アルファチャンネルを持たない画像はプログレッシブ JPEG（Quality: 85）に変換し、ファイル容量を **150〜300KB** 前後に抑制。
   - 透過 PNG/WebP は最適化 PNG として保持。
4. **CDN URL への自動置換 (デフォルト: jsDelivr)**:
   - Markdown 内の `![alt](filename.png "caption")` は、世界中（Cloudflareエッジ）に分散キャッシュされる **jsDelivr CDN** の絶対 URL に自動置換され、センタリングスタイルとキャプションが付与されます。
   ```text
   https://cdn.jsdelivr.net/gh/Masafuro/ADX@main/docs/blog/images/<最適化後のファイル名>
   ```
   - Hackaday.io のトップ掲載やニュースレターでアクセスが集中しても帯域無制限で耐えられます。
   - オプション指定（`--cdn raw` や `--cdn pages`）により、`raw.githubusercontent.com` や `masafuro.github.io` への切り替えも可能です。

---

## 5. ビルドスクリプト CLI リファレンス

スクリプト本体: `docs/blog/scripts/build_hackaday.py`

### コマンドオプション
```bash
python docs/blog/scripts/build_hackaday.py [OPTIONS]
```

| オプション | デフォルト | 説明 |
| :--- | :---: | :--- |
| `--date YYYY-MM-DD` | なし | 指定した日付の記事のみをビルドします（指定なし時は `posts/*.md` の全記事）。 |
| `--cdn {jsdelivr,raw,pages}` | `jsdelivr` | 画像ホスティングのプロバイダーを選択します。<br>・`jsdelivr`: jsDelivr CDN（高速・高耐久・推奨）<br>・`raw`: GitHub Raw（`raw.githubusercontent.com` 直結）<br>・`pages`: GitHub Pages（`masafuro.github.io`） |
| `--stdout` | なし | 生成された HTML スニペットを標準出力にも出力します。 |
| `--skip-images` | なし | 画像の最適化処理（`raw_assets/` → `images/`）をスキップし、Markdown の変換のみ行います。 |
| `-h`, `--help` | なし | ヘルプメッセージを表示します。 |

### 実行例
```bash
# デフォルト (jsDelivr CDN) で今日の記事をビルド
python docs/blog/scripts/build_hackaday.py --date 2026-10-22

# GitHub Raw 直結 URL で生成したい場合
python docs/blog/scripts/build_hackaday.py --date 2026-10-22 --cdn raw

# 原稿の文章だけ少し直して高速に再ビルド
python docs/blog/scripts/build_hackaday.py --date 2026-10-22 --skip-images
```

---

## 6. 生成される HTML スニペットの仕様 (`dist/`)

出力先: `docs/blog/dist/YYYY-MM-DD.html`

Hackaday.io のエディタは外部 CSS を読み込めず、素のタグではフォントや余白が崩れやすいため、以下のインライン装飾を施しています：

* **Hero 画像 & リード文**:
  * センタリング、微小なドロップシャドウ（`box-shadow: 0 4px 16px rgba(...)`）、太字リード文。
* **見出し (`<h2>`)**:
  * ADX シアンカラーの下線ボーダー付きスタイル（`border-bottom: 2px solid #00f0ff`）。
* **コードブロック (`<pre><code>`)**:
  * ダークテーマ背景（`#1e1e1e`）、等幅フォント、角丸、横スクロール対応。
* **引用ブロック (`<blockquote>`)**:
  * エメラルドグリーンのアクセントバー（`border-left: 4px solid #10b981`）と薄い背景色。
* **フッター (CTA)**:
  * ADX WebSerial Explorer へのライブリンク、GitHub リポジトリへのリンク、ドキュメントリンクを含む統一カード。

---

## 7. 動作要件・トラブルシューティング

### 必要な Python パッケージ
ビルドスクリプトは標準ライブラリのみでもフォールバック動作しますが、完全な画像最適化とリッチなMarkdown変換のために以下を推奨します：

```bash
pip install Pillow markdown
```

* **Pillow が無い場合**: 画像最適化は行われず、ファイルがそのまま `images/` にコピーされます。
* **markdown が無い場合**: 簡易な内蔵パーサーで基本タグ（見出し・リスト・引用）のみ変換されます。

### 画像が Hackaday.io で表示されない場合
* 画像はデフォルトで **jsDelivr CDN** (`https://cdn.jsdelivr.net/gh/Masafuro/ADX@main/docs/blog/images/...`) を参照しています。
* 記事を公開する前に、最適化された `docs/blog/images/` の画像が `git add docs/blog/images/` され、GitHub の `main` ブランチに `git push` 完了していることを確認してください。
* （プッシュ直後数分で CDN キャッシュが有効化されます）
