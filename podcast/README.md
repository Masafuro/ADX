# ADX Podcast / Audio Contents Workspace

`docs/` 配下の技術ドキュメント・ブログ（Hackaday.io DevLog）とは完全に独立した、音声ファースト（Podcast / YouTube）コンテンツ制作区画です。

## ディレクトリ構成

```text
podcast/
├── README.md                 # 本ドキュメント（区画案内）
└── contents/
    ├── v1/                   # 【試作】2人掛け合い型（DEV vs ECHO）
    │   ├── talk_show_bible.md
    │   ├── recent_dev_summary.md
    │   └── script_*.md
    ├── v2/                   # 【試作】ECHO独白型（仮: ECHO’s Audit Log）
    │   ├── show_bible.md
    │   └── episodes/
    ├── v3/                   # 【試作】DEV独白＋ECHO引用型
    │   ├── show_bible.md
    │   └── episodes/
    └── v4/                   # 【最新案】"I"-to-"You" 直接語り・演説型（架空AI設定なし）
        ├── show_bible.md
        └── episodes/
```

## バージョン変遷
* **v1 (`contents/v1/`)**: 現場エンジニア `DEV` と AI `ECHO` の2人掛け合い形式。
* **v2 (`contents/v2/`)**: `ECHO` が監査ログを単独朗読し、`DEV` をコミット引用として扱う独白形式。
* **v3 (`contents/v3/`)**: `DEV` が一人称で語りつつ、背景の監査AI `ECHO` の指摘をネタとして引用する形式。
* **v4 (`contents/v4/`)**: 架空のAI設定を外し、語り手（`"I"`）が聴き手（`"You"`）の疑問を直接受け止めながら語り切る、英語圏ネイティブな演説・直接対話型。
