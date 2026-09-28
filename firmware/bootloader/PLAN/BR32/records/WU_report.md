<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BR32 Warm Up (WU) 総合実証レポート: 実機検証から導かれた黄金の動作条件パターン

**最終更新日**: 2026-09-28  
**対象ハードウェア**: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)  
**配線環境**: 50cm (25cm+25cm WAGO 差込形コネクタ中継・3線 A/B/GND・両端 120Ω 終端抵抗 ON)  
**対象サンドボックス**: WU-0 〜 WU-4 (随時追記・更新)

---

## 1. エグゼクティブサマリ

本レポートは、机上の理論や推測を一切排除し、**実際のハードウェア（ADX Core-D / ATtiny1616 / SP485EEN）、市販 USB-RS485 ドングル、および現場レベルの実配線（WAGO 差込形コネクタ中継・終端抵抗）において、数千回に及ぶ実測ベンチマークによって証明された「動かぬ真実（Proven Facts）」と「黄金の動作条件パターン」** を体系的にまとめた技術規範である。

過去の OneToOne_RS485 開発で苦戦した「CRC エラー」「タイムアウト」「自爆エコーによるフリーズ」「ジッターの増大」といった問題は、本サンドボックス群の徹底的な実証により、その物理・数学・OS 的メカニズムが完全に解明され、再現性 100.00% の堅牢な動作パターンへと昇華された。

---

## 2. 確定された「黄金の動作条件パターン (The Golden Standard)」

以降の全 BR32 ツールチェーン（ホスト側 Python ツール、本番 Flasher、マイコン側ブートローダー、アプリケーション通信）において、以下の設計条件を**必須の確定標準**とする。

| 分類 | パラメータ / 項目 | 確定された黄金仕様 | 技術的根拠・理由 |
| :--- | :--- | :--- | :--- |
| **物理配線** | **信号線構成** | **3線結線 (A, B, GND)** | PC ドングルとマイコンの Common GND を直結することで、コモンモード電位差がゼロになり、トランシーバの過渡応答が俊敏化して RTT が約 0.7ms 軽快化する。 |
| **物理配線** | **終端抵抗** | **両端 120Ω ON (合成 60Ω)** | 波形の反射波・リンギングを完全にダンピング。200ms 周期における変動幅 Range を 3.29ms に圧縮。 |
| **Break 生成** | **PC 側送出手法** | **Method 1: Half-Baud Trick** | 19,200 bps の半速（9,600 bps）に切り替えて `0x00` 送出 $\rightarrow$ 19,200 bps 復帰。純粋なハードウェア波形で 18〜20 bit の正確な LOW パルスを射出（成功率 100.0% / `send_break()` は 0.0% で全滅）。 |
| **同期方式** | **LINAUTO 校正** | **BREAK + 0x55 (Sync)** | マイコンの LINAUTO ハードウェアが `0x55` を自動消費してボーレートをナノ秒単位で実測校正。通信クロックの熱ドリフト誤差を完全解消。 |
| **通信周期** | **推奨ポーリング周期** | **200.0 ms (5.0 Hz)** | Windows OS タイマー刻み（15.625ms）と綺麗に同期し、USB 省電力遷移（>300ms）も回避。ジッター $\sigma = 0.48\,\text{ms}$、変動幅 3.29ms、94.8% が ±1.0ms に集中する最高峰の決定性（GRADE A+）。 |
| **誤り訂正** | **CRC-16 計算方式** | **CRC-16-CCITT / XMODEM**<br>(Poly: `0x1021`, Init: `0xFFFF`, **MSB-first**) | Python 側とマイコン側（`<util/crc16.h>` の `_crc_xmodem_update`）で数学的に 1 ビットの狂いもなく完全一致（1,000 サイクルで CRC エラー 0 件 / 0.00%）。※`_crc_ccitt_update` は LSB-first のため絶対使用禁止。 |
| **フレーム長** | **プロトコル構造** | **完全 32 バイト固定長** | Master 送信: `[0x55 Sync] + [32B Master Frame]` (計 33B)<br>Slave 返信: `[32B Slave Frame]` (計 32B)<br>可変長を完全排除することでフレーミングズレを物理的に根絶。 |
| **回線制御** | **自爆エコー完全遮断** | **送信時 `/RE=1` 物理切断** | 送信前: `DE=1`, `/RE=1` $\rightarrow$ 50µs ディレイ<br>送信後: `TXCIF` 待機 $\rightarrow$ `DE=0`, `/RE=0` 復帰 $\rightarrow$ 受信 FIFO 空読みフラッシュ。自爆エコーの混入によるフリーズ・誤判定を 100% 遮断。 |
| **宛先照合** | **SIGROW ゲート照合** | **全0x00 または 自身UID のみ応答** | 宛先不一致（他人のUID、1bit反転、全0xFF、ランダムゴミ）時は **DE=0 のまま 1 ビットも喋らず 100% 完全沈黙（0 Bytes 送出）**。共有バスでのパケット衝突を物理的に根絶。 |
| **耐ノイズ性** | **無状態自己治癒力** | **LIN Break による強制リセット** | 50〜100回の連続ゴミパケット攻撃後も、次フレームの LIN Break で受信ステートマシンが即座に自律復帰（デッドロック発生率 0.00%、復帰遅延 RTT 38.70ms）。 |
| **応答設計** | **レスポンス優先度** | **RS-485 応答を最優先射出** | Soft-UART 等のデバッグ処理は返信完了後に回す。19,200 bps でのワイヤ伝送理論限界（約 34ms）に迫る **真の RTT: 38.60 ms** を達成。 |

---

## 3. CRC-16 計算方式の完全統一仕様

過去に「CRC が合わない」と苦しんだ最大の原因は、**ビットシフト方向（MSB-first vs LSB-first）の食い違い** にあった。本プロトコルでは以下のアルゴリズムを公式標準とする。

### 3.1 数学的仕様
* **多項式 (Polynomial)**: $x^{16} + x^{12} + x^{5} + 1$ (`0x1021`)
* **初期値 (Initial Value)**: `0xFFFF`
* **ビット方向**: **MSB-first (上位ビット優先 / 非リフレクト)**
* **出力 XOR**: `0x0000` (なし)

### 3.2 各言語・環境での実装コード

#### Python 実装 (`wu1_echo_bench.py` / `wu2_fuzz_bench.py` / ホスト Flasher)
```python
def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    """Calculate CRC-16-CCITT / XMODEM (Poly 0x1021, Init 0xFFFF, MSB-first)."""
    crc = init
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc
```

#### AVR C 実装 (`wu1_echo.c` / `wu2_fuzz.c` / ブートローダー本体)
```c
#include <util/crc16.h>

// 重要: 必ず _crc_xmodem_update を使用すること！
// (_crc_ccitt_update は LSB-first のため結果が逆転して不一致になる)
static uint16_t calc_crc16(const uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc = _crc_xmodem_update(crc, data[i]);
    }
    return crc;
}
```

* **検算エビデンス**:
  テストペイロード `01 01 00 00 ...` (30バイト) に対し、Python / AVR ともに **`0xEB60`** を算出して 1 ビットの狂いもなく完全一致した。

---

## 4. サンドボックス別 検証結果アーカイブ

### 4.1 【WU-0】PC 側 RS-485 BREAK 生成実験ベンチ
* **レポート詳細**: [`records/WU_sandboxes/WU0_break_generator.md`](./WU_sandboxes/WU0_break_generator.md)
* **主要成果**:
  1. PC からの LIN Break 生成 3 大手法（Half-Baud Trick, `send_break()`, 通常0x00）を比較し、**Half-Baud Trick（19200bps $\rightarrow$ 9600bps 0x00 $\rightarrow$ 19200bps 復帰）が 30/30 (100.0%) PASS**、時間ジッター $\sigma = 0.57\,\text{ms}$ で圧勝（他手法は 0.0% で全滅）。
  2. 1,000 回連続テスト（10cm 直結）において、**1,000/1,000 (100.00%) パス、ジッター $\sigma = 0.63\,\text{ms}$** を記録。
  3. 「最適インターバル検査くん（`--optimize`）」により、Windows のシステムタイマー基本刻み（約 15.625ms）との位相干渉（うなり現像）を可視化。

### 4.2 【WU-1】32B 固定長フレーム送受信 ＆ エコーバック統計実証
* **レポート詳細**: [`records/WU_sandboxes/WU1_echo_telemetry.md`](./WU_sandboxes/WU1_echo_telemetry.md)
* **主要成果**:
  1. LINAUTO の「Sync（0x55）自動消費・破棄」仕様を解明。Master は `[0x55] + [32B Frame]` (計33B) を送出し、マイコン側は完全 32 バイト（Byte 0..31）を受信・CRC 検証する構造を確立。
  2. CRC-16 計算方式（`_crc_xmodem_update`）の完全一致により、**1,000 回長周期テスト（200ms 周期 / 50cm WAGO中継配線）で 1,000/1,000 (100.00%) 完全完走、パケットロスゼロ、スレーブシーケンス欠落 0** を達成。
  3. マイコン内部の Signature Row から **生 SIGROW（10バイト UID: `0x30 53 51 46 33 34 29 29 14 21`）** が 1,000 回すべて正確に開示・返信され、自己同定機構が完璧に機能することを証明。
  4. 往復遅延 RTT は **平均 38.60 ms**（ワイヤ伝送理論限界値と一致）、変動幅 Range は **わずか 3.29 ms**、中心 ±1.0ms に **94.8% が集中** する最高評価「GRADE A+」を獲得。

### 4.3 【WU-2】BR32 SIGROW ゲート・チェッカー ＆ UID ファジング実機実証
* **レポート詳細**: [`records/WU_sandboxes/WU2_sigrow_fuzzing.md`](./WU_sandboxes/WU2_sigrow_fuzzing.md)
* **主要成果**:
  1. **鉄壁の沈黙（Complete Bus Silence）の実証**:
     宛先不一致の 4 大意地悪パケット（1bit反転、全0xFF、他人UID、ランダムゴミUID）に対し、スレーブが **バス上に 1 ビットも喋らず 100.0% 完全沈黙（0 Bytes 送出）を守り抜くこと** を実証。マルチドロップバス衝突防止の絶対的信頼性を確立。
  2. **50 連続ファジング攻撃 ＆ 一撃即時自己治癒（Zero Deadlock）**:
     50 回の高速ランダムパケット連打に対して 50/50 完璧に沈黙し、直後の正常パケットに対して **わずか 38.70 ms で平然と一撃即時復帰**。LIN Break によるステートマシンの強制自律リセット機構が証明された。
  3. **仕様書 3.2 節（正式スレーブ返信フレーム）の完全動作**:
     `STATUS`, `ECHO_SEQ`, `RX_COUNT`, `DEV_STATE`, `MY_SIGROW[10]`, `COUNTER`, `EXTRA[14]`, `CRC16` の全フィールドが実機で正確に往復通信できることを確認（GRADE A+ 獲得）。

---

## 5. 未解明課題と今後の調査テーマ

実運用上は「200ms 周期」を採用することで完全に回避可能であるが、将来の学術的深掘り課題として以下を記録・保管している：

* **研究課題 1: 長周期ポーリング（> 300ms）における系統的ジッター悪化の謎**:
  ポーリング周期を 300ms 超に遅くすると、直感に反してジッターが 0.65〜0.81ms、Range が 3.2〜4.0ms へと系統的・安定的に悪化する現象。
  - 保管先: [`records/open_questions.md`](./open_questions.md)
  - 有力仮説: USB ドングルの省電力サスペンド遷移準備タイマー、および線路の長時間アイドルによる過渡立ち下がりエッジ揺らぎ。

---

## 6. 今後追記予定の検証テーマ (Roadmap)

1. **【WU-3】フレーム破壊 ＆ ノイズ耐性実験室 (Fault Injection Lab)**:
   - 途中切断（10B 欠損）、過剰データ（50B 垂れ流し）、CRC 破壊に対する時間枠自律脱出（非デッドロック）の実証。
2. **【WU-4】仮想 SRAM ページペインタ (Virtual Page Painter)**:
   - Flash 書込なしで、SRAM 上の 64B 配列に 4 チャンク分割蓄積・読出遊び。
3. **【M1〜M5】本番ブートローダー開発マイルストーン**:
   - 1024 バイト以内への凝縮、単一 Flash ページ書き込み（M4）、フルアプリ OTW 書込（M5）。

