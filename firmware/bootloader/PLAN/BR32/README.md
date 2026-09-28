# BR32 (LIN Break + RS-485 32-Byte Fixed Frame Bootloader)

## 1. プロジェクト概要

**BR32** は、ADX Core-D（Microchip ATtiny1616-MNR）向けに新規設計された、**フィールドネットワーク（RS-485 半二重マルチドロップバス）に特化した超堅牢・極小ブートローダー** です。

従来の可変長パケットや複雑なハンドシェイクを全廃し、**「LIN Break によるハードウェア同期 ＋ 32 バイト固定長フレーム ＋ 10 バイト生 UID 宛先照合 ＋ 1,024 バイト極小フットプリント」** により、産業グレードの決定論的通信と耐ノイズ性を実現しています。

---

## 2. BR32 黄金の 6 大原則 (The 6 Fundamental Axioms of BR32)

車載 LIN プロトコルが 12V 単線ノイズ環境を 8B 固定長で克服したのと同様に、BR32 は以下の 6 大原則により「頭（LIN Break）」と「お尻（原則 6）」でフレーム前後を完全密閉しています：

1. **原則 1 (LIN BREAK 同期)**: LIN Break（$\ge 13$ bits LOW）検出でハードウェア強制ゼロクリア、32B 時間枠の受信を開始。
2. **原則 2 (完全沈黙)**: BREAK なし、または宛先 UID 不一致の通信に対しては **1 ビットも発言せず完全沈黙（DE=0 維持）**。
3. **原則 3 (欠損検知)**: BREAK あり、32B 未満で途絶えた場合は **`STATUS_ERR_TIMEOUT (0x02)` と実受信バイト数（`RX_COUNT`）を正直に返信**。
4. **原則 4 (CRC 異常通知)**: BREAK あり、32B 受信、CRC 不一致の場合は **`STATUS_ERR_CRC (0x01)` を返信**。
5. **原則 5 (正常受託)**: BREAK あり、32B 受信、CRC 一致の場合は **`STATUS_OK (0x00)` を即座に返信**。
6. **原則 6 (超過沈黙)**: BREAK あり、32B 満了後も後続バイトが継続している場合は **原則 2 相当（ノイズ・フレーム崩壊）として即時完全沈黙**（連鎖衝突の物理的防止）。

---

## 3. ディレクトリ構成と開発ロードマップ

```text
firmware/bootloader/PLAN/BR32/
├── README.md                          # 本ドキュメント（総合案内）
├── concept.md                         # 概念設計書
├── specification.md                   # 32 バイト固定フレーム詳細仕様書
├── milestones.md                      # 全マイルストーン計画書 ＆ 合格基準
│
├── WU/                                # Warm Up サンドボックス群 (全 5 段階完全制覇)
│   ├── README.md                      # WU 全体概要
│   ├── WU0_break_generator/           # WU-0: Half-Baud Trick による LIN Break 安定生成 (1,000/1,000 成功)
│   ├── WU1_echo_telemetry/            # WU-1: 32B 固定長 Ping-Pong ＆ 生 UID 自己同定 (ジッター σ=0.48ms)
│   ├── WU2_sigrow_fuzzing/            # WU-2: SIGROW 宛先照合ゲート ＆ 50 連打ファジング即時治癒
│   ├── WU3_fault_injection/           # WU-3: フレーム破壊注入 ＆ 原則 6 超過沈黙の確立
│   └── WU4_sram_painter/              # WU-4: 16B x 4 チャンクによる 64B 仮想 SRAM 蓄積・100% 一致
│
├── M4_single_page_write/              # Milestone 4: 単一 Flash 書込初期検証 (1506B)
│   ├── README.md                      # 初代 M4 手順書
│   ├── m4_flash.c                     # スレーブ側 C ソース
│   ├── m4_flash_bench.py              # ホスト側 Python ベンチマーク
│   └── (成果: ハードウェア SPM アンロック成功と、実行コード自己上書き自爆現象を特定)
│
├── M4_1_compact_bootloader/           # Milestone 4-1: 極小ブートローダー (1022B) ★GRADE A+ 達成★
│   ├── README.md                      # M4-1 手順書
│   ├── m4_1_flash.c                   # 1,022B 極小ブートローダー本体 (Pages 0..15 に完全収容)
│   ├── m4_1_flash_bench.py            # M4-1 Python ベンチマーク
│   └── (成果: 自爆ゼロ、物理 Flash 64B 全 512 ビット 100% 一致、保護領域鉄壁ガード)
│
└── records/                           # 実機検証エビデンス・ログアーカイブ
    ├── WU_report.md                   # Warm Up 全制覇レポート (GRADE A+)
    ├── WU_sandboxes/                  # WU-0 〜 WU-4 個別実機ログ
    ├── M4_single_page_write/result.md # M4 初期ログ（自爆現象の特定とナレッジ）
    └── M4_1_compact_bootloader/result.md # M4-1 公式エビデンス（100% Bit-for-Bit PASS）
```

---

## 4. マイルストーン進捗状況

| Milestone | 検証テーマ | 達成状況 | 備考 |
| :---: | :--- | :---: | :--- |
| **M0** | 3 ポート環境認識 (COM19/20/21) | **PASS** | ハードウェア接続完了 |
| **M1** | 32B Ping-Pong ＆ 生 UID 自己同定 | **PASS** | WU-1 にて 1,000 サイクル完走、ジッター $\sigma=0.48\,\text{ms}$ |
| **M2** | SIGROW 宛先照合 ＆ 完全沈黙 | **PASS** | WU-2 にて宛先不一致時の 100% 沈黙と即時治癒実証 |
| **M3** | 64B SRAM 蓄積 ＆ 分割読出ベリファイ | **PASS** | WU-4 にて 16B $\times$ 4 チャンク全ビット 100% 一致 |
| **M4** | 単一 Flash ページ書込 (初期検証) | **PASS** | ハードウェア SPM アンロック成功 ＆ 自爆課題の特定 |
| **M4-1**| **極小ブートローダー (<1024B) Flash 書込** | **GRADE A+** | **1,022B 収容、自爆ゼロ、物理 Flash 64B 100% 一致** |
| **M5** | **全 Flash (15KB) OTW 書込 ＆ 自動起動** | **★ 次期 Active Gate ★** | M4-1 コアを用い、実スケッチ（白色 LED 点滅）の OTW 書込 |
| **M6** | バスクロック短縮ストレステスト | 計画中 | 200ms $\rightarrow$ 100ms $\rightarrow$ 50ms 限界特定 |

---

## 5. ハードウェア確定ピンアサイン (ADX Core-D)

* **RS-485 (USART0 Alternate)**:
  - `PA1`: TXD
  - `PA2`: RXD
  - `PA4`: SP485EEN DE (Active HIGH)
  - `PA7`: SP485EEN /RE (Active LOW)
* **インジケータ**:
  - `PB2`: オンボード赤色 LED（ブートローダー待機 2Hz ハートビート ＆ 通信トグル）
  - `PB3`: オンボード白色 LED（ユーザーアプリケーション稼働表示）
* **デバッグ / 書き込み**:
  - `PB4`: Soft-UART TXD (9,600 bps / CH342K Port B / COM21)
  - `UPDI`: SerialUPDI (CH342K Port A / COM20)
