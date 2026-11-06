# ARLE.cc Brand Website

Official brand website for **ARLE.cc** (https://arle.cc).

## 1. Brand Hierarchy

```text
┌─────────────────────────────────────────────────────────────────┐
│ 【 運営母体・法的主体 】                                         │
│  古川企画 (Furukawa Kikaku) / furukawakikaku.jp                  │
│  ・日本・長崎・佐世保の組込システム開発ファーム                  │
│  ・国内B2B受託開発、回路・基板・ファームウェアの受託設計         │
└────────────────────────────────┬────────────────────────────────┘
                                 │ 自社グローバル・プロダクト部門
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│ 【 世界向けメーカーブランド 】                                   │
│  ARLE.cc (https://arle.cc)                                      │
│  ・世界市場（英語圏メイカー、組込エンジニア）向けブランド        │
│  ・「現場で本当に動くハードウェア」を生み出す独立デザインレーベル│
└────────────────────────────────┬────────────────────────────────┘
                                 │ 第1弾フラッグシップ規格＆製品群
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│ 【 オープン規格 ＆ プロダクトエコシステム 】                     │
│  ADX (Advanced Devices eXtended)                                │
│  ・"The Working Arduino" / 8748 フォームファクタ規格             │
│  ・CORE-U / CORE-A / CARD シリーズ / MR32 通信規格              │
└─────────────────────────────────────────────────────────────────┘
```

## 2. Directory Structure

```text
site/
├── index.html            # Main landing page entry point
├── public/               # Static assets directly served at web root
│   └── assets/
│       └── logos/        # ARLE.cc, ADX, Furukawa Kikaku logos
├── src/                  # Source files for components and styling
│   ├── components/       # Reusable UI sections (Hero, Features, Showcase)
│   └── pages/            # Multi-page layouts (future expansion)
└── README.md             # This document
```

## 3. Deployment

This directory is designed to be deployed directly as the root of the `arle.cc` website:

- **Cloudflare Pages / Vercel / Netlify**:
  - Root directory: `site`
  - Build command: (None for static HTML, or `npm run build` if using SSG/Vite)
  - Output directory: `.` (or `dist` if built)
- **Domain**: `arle.cc`

