# Milestone 4 (M4): 単一 Flash ページ消去・書き込み ＆ 非同期分離実証

## 1. 概要と目的
Warm Up（WU-0 〜 WU-4）の全制覇により確立された **BR32 黄金の 6 大原則** および **非同期分離シーケンス** に基づき、初めて ATtiny1616 の **物理 Flash メモリ（`0x0400` / Page 16）** への消去・書き込み（Erase & Write）を実施します。

### 本マイルストーンの核心ポイント
1. **非同期分離 (Asynchronous Response Isolation)**:
   - Chunk 3（4 チャンク目・コミット指示）受信後、スレーブは **まず即座に RS-485 応答（ACK）を返信**（約 16.6ms）。
   - 送信完了（`USART_TXCIF_bm` 検出）直後に、マイコン内部で **Flash 消去書き込み（25ms）を実行**。
   - 次サイクル（200ms）まで **約 158ms のスラックタイム** があるため、マスター側の特別なビジーポーリング待ちすら不要で安全に完結。
2. **物理 Flash からの直接読み出しベリファイ**:
   - `CMD_READ_CHUNK` は SRAM ではなく **マイコン物理 Flash（`MAPPED_PROGMEM_START + 0x0400`）から直接 16 バイトずつ読み戻し**。
   - 64 バイト全ビット（100% Bit-for-Bit）が完全一致することを実証。
3. **ブートローダー保護領域ガード**:
   - アドレス `0x0000`〜`0x03FF`（Page 0〜15）への書き込み要求は `STATUS_ERR_PROTECT (0x03)` で物理的に拒絶。

---

## 2. 実行手順 (Windows PowerShell)

### Step 1: UPDI によるファームウェア書き込み (COM20)
```powershell
cd firmware/bootloader/PLAN/BR32/M4_single_page_write
pymcuprog write -d attiny1616 -t uart -u COM20 -f m4_flash.hex --erase --verify
```

### Step 2: M4 ベンチマーク ＆ ベリファイ実行 (COM19 & COM21)
```powershell
python .\m4_flash_bench.py --port COM19 --debug-port COM21
```

---

## 3. 検証項目と合否判定基準 (GRADE A+)

| 項目 | 検証内容 | 合格基準 |
| :--- | :--- | :--- |
| **1. 自己同定** | `CMD_IDENTIFY` によるスレーブ検出 | スレーブ既知 UID（`0x30 53 51 46 33 34 29 29 14 21`）の取得 |
| **2. ベースライン読出** | 書き込み前の Page 16 読み出し | `CMD_READ_CHUNK` で 64B 正常取得 |
| **3. 4チャンク書込** | Page 16 への 16B $\times$ 4 書込 ＆ 即時 ACK | 4 チャンクすべて `STATUS_OK` ＆ エコー一致 |
| **4. 非同期 Flash コミット** | ACK 送出直後の 25ms 消去書き込み | マイコンフリーズ・バス破壊ゼロ、次サイクル即応 |
| **5. 物理 Flash ベリファイ** | 物理 Flash からの 64B 読出照合 | **64/64 バイト 100% 完全一致 (Bit-for-Bit Match)** |
| **6. 保護領域ガード** | Page 0 (0x0000) への書き込み拒絶 | **`STATUS_ERR_PROTECT (0x03)` 返信** |
