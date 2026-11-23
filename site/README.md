<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ARLE.cc Website & ADX Documentation Portal

Official website and documentation portal for **[ARLE.cc](https://arle.cc)** and the **ADX** open-hardware ecosystem, built with **Astro + Starlight** and deployed on **Cloudflare Pages**.

## 1. Local Development

Requires **Node.js `>= 20.3.0`** (tested on Node 20 LTS).

```bash
cd site
npm install
npm run dev      # Start local dev server (http://localhost:4321)
npm run build    # Build static site + Pagefind search index into dist/
npm run preview  # Preview the production dist/ build locally
```

## 2. Structure & Localization

- **Default Locale (English)**: `/` and `/adx/...` (`src/content/docs/`)
- **Japanese Locale**: `/ja/` and `/ja/adx/...` (`src/content/docs/ja/`)
- **Untranslated Fallback**: English-only SSOT pages (e.g. `/adx/standards/`) automatically fall back cleanly under `/ja/` without returning 404.

```text
site/
├── astro.config.mjs          # Astro + Starlight configuration (i18n, sidebar, logo)
├── package.json              # Scripts & dependencies (Astro 5 + Starlight 0.37)
├── public/
│   ├── favicon.svg           # ARLE.cc SVG favicon
│   └── assets/logos/         # Public static brand logos
└── src/
    ├── assets/logos/         # Brand logos processed by Astro
    ├── content.config.ts     # Starlight Content Layer collection definition
    ├── styles/custom.css     # ARLE.cc industrial dark/light theme overrides
    └── content/docs/         # EN (root) & JA (ja/) documentation pages
```

## 3. Cloudflare Pages Deployment

Configure the `arle.cc` Cloudflare Pages project (**Settings → Build**) as follows:

| Setting | Value |
| :--- | :--- |
| **Root directory** | `site` |
| **Build command** | `npm run build` |
| **Build output directory** | `dist` |
| **Environment variable** | `NODE_VERSION = 20` |
| **Build watch paths (Include)** | `site/*`, `docs/*` |

- Including `docs/*` ensures future SSOT specification updates in `docs/` automatically trigger a rebuild while ignoring unrelated `memo/`, `hardware/`, `firmware/`, and `podcast/` commits.
- Architecture & milestone roadmap: [`memo/PLAN/ARLE_STARLIGHT_SITE_ARCHITECTURE_PLAN.md`](../memo/PLAN/ARLE_STARLIGHT_SITE_ARCHITECTURE_PLAN.md)
