# ADX Podcast / Audio Contents Workspace

`docs/` 配下の技術ドキュメント・ブログ（Hackaday.io DevLog）とは完全に独立した、音声ファースト（Podcast / YouTube）コンテンツ制作区画です。

## ディレクトリ構成

```text
podcast/
├── README.md                 # 本ドキュメント（区画案内）
└── contents/
    ├── show_bible.md         # 【採用版: v5】2段階制作パイプライン設計書（v1対話抽出 → v4演説変換）
    ├── drafts/               # Stage 1: v1対話によるネタ出し・反論・比喩抽出ドラフト
    ├── episodes/             # Stage 2: v4形式（"I"-to-"You" @DEV:）の本番台本
    └── archive/              # 検討過程のアーカイブ（v1〜v4）
        ├── v1/               # 2人掛け合い型（DEV vs ECHO）
        ├── v2/               # ECHO独白型
        ├── v3/               # DEV独白＋ECHO引用型
        └── v4/               # 直接語り型（単体運用版）
```

## 現行ワークフロー（v5採用）
1. **Stage 1 (`drafts/`)**: 開発トピックを `DEV vs ECHO` の対話で揉み、禁忌フック・反論・耳でわかる比喩・数値オチを抽出する。
2. **Stage 2 (`episodes/`)**: 5つの変換ルールに沿って `ECHO` を聴き手（`"You"`）へ溶かし込み、`@DEV:` の演説的モノローグ台本へ仕上げる。
