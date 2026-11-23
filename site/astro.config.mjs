// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
  site: 'https://arle.cc',
  integrations: [
    starlight({
      title: 'ARLE.cc',
      description:
        'Hardware engineered for the real world. Official documentation and open-hardware standards for ARLE.cc and the ADX ecosystem.',
      favicon: '/favicon.svg',
      logo: {
        src: './src/assets/logos/arle_logo.png',
        alt: 'ARLE.cc',
        replacesTitle: true,
      },
      components: {
        ThemeProvider: './src/components/ThemeProvider.astro',
        ThemeSelect: './src/components/ThemeSelect.astro',
      },
      defaultLocale: 'root',
      locales: {
        root: {
          label: 'English',
          lang: 'en',
        },
        ja: {
          label: '日本語',
          lang: 'ja',
        },
      },
      social: [
        {
          icon: 'github',
          label: 'GitHub',
          href: 'https://github.com/Masafuro/ADX',
        },
        {
          icon: 'external',
          label: 'Hackaday.io',
          href: 'https://hackaday.io/project/205547-adx-enclosure-native-modular-hardware-standard',
        },
        {
          icon: 'rss',
          label: 'Off the Breadboard (Spotify Podcast)',
          href: 'https://open.spotify.com/show/79cvRtpnEFaMkGB9mDjPpd',
        },
      ],
      editLink: {
        baseUrl: 'https://github.com/Masafuro/ADX/edit/main/site/',
      },
      customCss: ['./src/styles/custom.css'],
      head: [
        {
          tag: 'link',
          attrs: {
            rel: 'preconnect',
            href: 'https://fonts.googleapis.com',
          },
        },
        {
          tag: 'link',
          attrs: {
            rel: 'preconnect',
            href: 'https://fonts.gstatic.com',
            crossorigin: '',
          },
        },
        {
          tag: 'link',
          attrs: {
            rel: 'stylesheet',
            href: 'https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500;600;700&display=swap',
          },
        },
      ],
      sidebar: [
        {
          label: 'ADX Ecosystem',
          translations: { ja: 'ADX エコシステム' },
          items: [
            {
              label: 'Overview',
              translations: { ja: '概要（ADX とは）' },
              slug: 'adx',
            },
            {
              label: 'Standards Snapshot (SSOT)',
              translations: { ja: '規格サマリー（英語 SSOT）' },
              slug: 'adx/standards',
            },
          ],
        },
      ],
    }),
  ],
});
