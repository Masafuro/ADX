<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# 産業用フィールドネットワーク DROP-Bus 仕様書・ホワイトペーパー

**DROP-Bus** (**D**eterministic **R**elay with **O**ptional-mediator & **P**assive-failsafe Bus)

---

## 1. エグゼクティブサマリー＆設計思想 (Executive Summary & Core Philosophy)

### 1.1 背景と課題（既存フィールドバスの限界）
現代の産業用・組み込み用分散制御システム（多軸ロボット、搬送ライン、センサ・アクチュエータネットワーク等）において、リアルタイム通信プロトコルの選定は常に厳しいトレードオフに直面してきました。

* **CAN / CAN-FD:** 優先度調停（CSMA/CD）と自動再送機構を内包するため、高負荷時やノイズ発生時に通信ジッターが肥大化し、最悪実行時間（WCET: Worst-Case Execution Time）の見積もりが極めて困難。
* **LIN:** 低コストかつオートボーレート同期を備えるが、マスター・スレーブ依存が強く（SPOFリスク）、データ長が最大8バイト、通信速度が最大19.2 kbpsと制御用途には帯域不足。
* **Modbus-RTU / RS-485 ポーリング:** 呼出・応答（Poll/Response）による通信オーバーヘッドが大きく、バス利用効率が低い。
* **一般的なトークンリング / 産業用イーサネット:** 高速かつ決定論的であるが、各ノードに高精度な水晶発振器や専用コントローラIC（PHY/MAC）が必須となり、ノード単価と基板面積を押し上げる。

DROP-Bus は、これらの課題を根本から打破するために**「極限の低コスト（単一SKU・完全水晶レス）」**、**「自律分散 Pub/Sub（Unique ID / Common Subscriber）」**、**「厳格な決定論性（ジッター拘束）」**、そして**「受動的フェイルセーフ（沈黙＝安全）」** を四位一体で実現する新しい自律分散型フィールドバス規格です。

---

### 1.2 コア設計思想

```text
=============================================================================
                      DROP-Bus の 4大コア哲学
=============================================================================
 1. 沈黙は安全 (Silence is Safety)
    - 通信途絶・パケット破損時は自律再送やスキップを一切行わない
    - バトンが落ちた瞬間に全ノードが同期して即時安全停止（Safe Torque Off）

 2. 自律分散 Pub/Sub (Unique ID / Common Subscriber)
    - IDは「物理デバイス」ではなく「論理バトン（スロット/トピック）」を指す
    - Unique Publisher: 各バトンIDはネットワーク内で唯一の物理ノードが所有
    - Common Subscriber: バス上に放流されたデータは全ノードが自由に傍受・購読

 3. メカニズムとポリシーの完全分離 (Separation of Mechanism & Policy)
    - プロトコル・コアは「バトンリレー」と「沈黙停止」のみを極小実装
    - 再送回数・介入・ジッターバジェットは上位ポリシー（Mediator / Plugin）に委ねる

 4. 単一SKU・完全水晶レス (Crystal-less Single SKU)
    - 末端ノードは内蔵RC発振器の低コストMCU（ATtiny1616等）で動作
    - フレーム先頭の LINAUTO (Break + 0x55) 物理同期により毎周クロックを自動校正
=============================================================================
```

1. **沈黙は安全（Silence is Safety）**
   * プロトコル層がブラックボックスで再送やノードスキップを試みる「勝手な継続動作」を全廃。
   * バトンが脱落（DROP）した場合、バスは無音となり、全ノードの内蔵ハードウェアタイマーが一斉に満了してアクチュエータ出力を安全遮断（STO: Safe Torque Off）します。
2. **自律分散 Pub/Sub（Unique ID / Common Subscriber モデル）**
   * IDを「物理ノードの固定シリアル」から**「論理バトンID（スロット / トピック識別子）」**へと昇華。
   * 1つの物理ノードが複数のバトンIDを所有することで、中央マスターなしに**不等周期サンプリング（Multi-rate Scheduling / TDMA的な時間スロット配分）**を実現。
   * 共有バス上に送出されたフレームは、全ノードが生存確認しつつ、必要なトピックを各自の受信メールボックス（Rx Mailbox）へ自由に購読（Subscribe）します。
3. **メカニズムとポリシーの完全分離（Decoupled Architecture）**
   * プロトコル・コア（Core）はバトンの受け渡しと受動的停止のみに徹します。
   * 再送の有無や介入タイミングは、外付けまたは内包された **Mediator（調停者）** および **再送プラグイン** の裁量下に置かれます。
4. **単一SKU・完全水晶レス（Crystal-less Single SKU）**
   * 全末端ノードが同一のハードウェア構成・ファームウェア（単一SKU）で動作可能です。
   * 高価な外付け水晶発振器を不要とし、フレーム単位の物理同期によって内蔵RCドリフトを完全に吸収します。

---

## 2. 用語定義と論理ロール (Terminology & Logical Roles)

### 2.1 コア用語

| 用語 | 英語表記 | 定義と役割 |
| :--- | :--- | :--- |
| **物理ノード** | Physical Node | ネットワークを構成するハードウェア実体。1つ以上のバトンID（Publisher）を所有し、同時に任意のバトンIDを購読（Subscriber）できる。 |
| **バトンID (論理スロット)** | Baton ID / Logical Slot | 発話権とデータトピックを表す論理識別子（`0x01` 〜 `0xFE`）。各バトンIDはネットワーク全体で**唯一の物理ノード（Unique Publisher）**に割り当てられる。 |
| **SYSTEM スロット** | SYSTEM Slot (`0xFF`) | Mediator や管理エンティティが使用する**システム予約スロット（ID: `0xFF`）**。起動時 Ping 点呼、定期診断、再点火、全体制御に用いる。 |
| **ユニーク・パブリッシャー** | Unique Publisher | 各バトンIDに対して唯一存在する送信権保持者。自身宛てのバトン（`TARGET_ID`）を受信した際、自身のデータを送出しつつ次バトンへパスする。 |
| **コモン・サブスクライバー** | Common Subscriber | 共有バス（RS-485）を流れる全フレームを傍受し、必要なバトンIDのペイロードを自律的に取得・購読する機能。 |
| **ドロップ** | Drop | 通信エラー、未応答、ノード脱落、または調停シグナルによってバトンが失われ、バスが完全な無音状態に陥る現象。 |
| **パッシブ・フェイルセーフ** | Passive Fail-Safe | バトンドロップ発生時、全ノードが再送を行わず、ハードウェアタイマーの満了によって同期して安全停止（STO）へ移行する保護動作。 |
| **メディエーター** | Mediator | システムに任意で配置可能な**調停者（論理ロール）**。SYSTEMスロット（`0xFF`）を行使し、平常時は無音監視、起動・異常・保守時のみ介入（Ping、衝突、再点火）を行う。 |

---

### 2.2 概念（論理ロール）としての Mediator

DROP-Bus における Mediator は「特定の専用ハードウェア」ではなく、**「落ちたバトンを拾い直す、またはバスの秩序を書き換える権能を持つ論理的な役割（Role）」** です。

```text
【形態1: 完全マスターレス構成 (No Mediator)】
[Node A (Baton: 1, 3)] <───(RS-485 2-Wire)───> [Node B (Baton: 2)]
※ 最安・最軽量構成。ドロップ時は全系が潔く即時安全停止し、外部リセットまで待機。

【形態2: ノード内包型 (Co-located Mediator)】
[Node A (Runner + Mediator)] <───(RS-485)───> [Node B (Runner)]
※ PLC直結ノードや処理能力に余裕のあるノードに SYSTEM (0xFF) 調停ロジックを同居させる。

【形態3: 外付けアタッチメント型 (Dedicated Mediator)】
[Node A] ─── [Node B] ─── (RS-485) ─── [ Dedicated Mediator (上位GW / 診断ロガー) ]
※ 末端は純粋な単一SKU。SYSTEM (0xFF) を用いた診断・上位連携用として別基板を追加。

【形態4: 保守ツール型 (Maintenance Dongle)】
[Node A] ─── [Node B] ─── [ (Service Port) <--- (Handheld Tool: SYSTEM 0xFF) ]
※ 通常時はマスターレスで稼働し、保守員が活線挿抜や構成変更を行う際のみ接続。
```

---

## 3. 物理層およびクロックレス同期設計 (Physical Layer & Synchronization)

### 3.1 物理層構成
* **伝送路:** RS-485 半二重 2線式差動伝送（A / B / GND）
* **トポロジー:** デイジーチェーン / マルチドロップ（スタブ長は極小化）
* **終端抵抗:** バス両端に 100〜120 $\Omega$（ADX Core-D 標準ジャンパ対応）
* **フェイルセーフバイアス:** バス解放時の不定電位・ノイズ誤検知を防ぐため、プルアップ/プルダウンによるフェイルセーフバイアスを推奨

```text
[Node 1]               [Node 2]               [Node N]
+--------+             +--------+             +--------+
| ATtiny |             | ATtiny |             | ATtiny |
|  1616  |             |  1616  |             |  1616  |
+---+----+             +---+----+             +---+----+
    | UART                 | UART                 | UART
+---+----+             +---+----+             +---+----+
| SP485  |             | SP485  |             | SP485  |
+---+----+             +---+----+             +---+----+
    | A/B                  | A/B                  | A/B
====+======================+======================+===== RS-485 Bus (2-Wire)
```

---

### 3.2 完全水晶レス設計（Crystal-less Operation）
DROP-Bus では、全末端ノードがマイコン内蔵の RC 発振器（ATtiny1616 内部 16MHz/20MHz オシレータ等）で駆動します。

* **短期周波数安定性:** 内蔵RC発振器は、1フレームの伝送時間（数百 $\mu\text{s}$ 〜 数 $\text{ms}$）内であれば、温度や電源電圧の急峻な変化を受けず、極めて高い短期周波数安定性を誇ります。
* **累積誤差のゼロリセット:** フレーム先頭で受信する同期シグナルごとにボーレート設定レジスタ（`BAUD`）を直接ハードウェア補正するため、長期的なクロックドリフトは完全に相殺されます。

---

### 3.3 LINAUTO 同期メカニズム

UART物理層には、車載LIN規格に準拠した **LINAUTO（Hardware Auto-Baudrate）** 方式を採用します。

```text
[DROP-Bus 物理伝送波形]
       |<--- Break Field --->| Delimiter |<----- Sync Byte ----->|
TX: ───┐                     ┌───────────┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─────┴─[ LEN ] [ TARGET_ID ] ...
       └─────────────────────┘           └─┘ └─┘ └─┘ └─┘ └─┘
       |<--- 13~14 bit 0 --->| (>=1bit 1)|<--- 0x55 (測定) ----->|
```

1. **Break Field 送出:**
   送信ノードは、フレーム先頭で 13〜14 ビット時間相当の連続 Low（Dominant）を送出します。
2. **Break 検出:**
   受信側マイコン（USART LINAUTO モード）は、11 ビット以上の連続 Low を検出すると「新フレーム開始」と判定し、ハードウェア計測タイマーを待機状態にします。
3. **Delimiter:**
   1 ビット以上の High（Recessive）期間を置き、立ち下がりエッジを明確化します。
4. **Sync Byte (`0x55`) による自動校正:**
   続く `0x55`（2進表記: `01010101`）のエッジ間隔をハードウェアタイマーが自動測定し、受信USARTのボーレートレジスタを送信側の実周波数に同期校正します。
5. **ペイロード受信:**
   後続するデータバイト列は、校正されたボーレートによってサンプリング誤差ゼロで確実に受信されます。

---

## 4. データフレーム仕様 (Data Frame Specification)

### 4.1 フレームフォーマット

最大64バイトの可変長ペイロードに対応し、高い誤り検出能力を持つ **CRC-16-CCITT** を採用します。

```text
|<------------- 物理同期ヘッダ ------------->|<------------------------- 論理データフレーム ------------------------->|
+---------+------+-------+-----+-----------+-----------+-----------------------------------+---------+
|  BREAK  | DEL  | SYNC  | LEN | TARGET_ID | SENDER_ID |       PAYLOAD (0 .. N Byte)       | CRC-16  |
+---------+------+-------+-----+-----------+-----------+-----------------------------------+---------+
  >=13bit   1bit    1B     1B       1B          1B              可変長 (最大64 Byte)              2B
```

---

### 4.2 8-bit ID 空間のゾーン定義

DROP-Bus は 8 ビット（`0x00` 〜 `0xFF` / 全256個）の ID 空間を以下のように割り当てます。

```text
+-------------------------------------------------------------------------------+
|                      DROP-Bus 8-bit ID 空間仕様 (0x00 〜 0xFF)                |
+-------------------------------------------------------------------------------+
| [0x00]              : 予約 (NULL / 未初期化 / アイドル)                        |
| [0x01 〜 0xFE] (1〜254): ユーザー通常バトン / トピックID (★計254個の広大な空間)   |
| [0xFF] (255)        : SYSTEM スロット (Mediator / Ping点呼 / 診断 / 再点火)    |
+-------------------------------------------------------------------------------+
```

| フィールド | サイズ | 設定値 / 範囲 | 説明 |
| :--- | :---: | :---: | :--- |
| **BREAK** | $\ge 13$ bit | Dominant (0) | ハードウェア LINAUTO トリガー用の連続 Low 信号 |
| **DEL** | $\ge 1$ bit | Recessive (1) | ブレーク・デリミタ（立ち上がり区間） |
| **SYNC** | 1 Byte | `0x55` | オートボーレート計測用の交番ビットパターン |
| **LEN** | 1 Byte | `0x00` 〜 `0x40` | 後続する PAYLOAD のバイト数 $N$（最大64バイト） |
| **TARGET_ID** | 1 Byte | `0x01` 〜 `0xFF` | 次に発話権を受け取る **論理バトンID**（`0xFF` = SYSTEM） |
| **SENDER_ID** | 1 Byte | `0x01` 〜 `0xFF` | 現在発話している **論理バトンID / トピックID**（`0xFF` = SYSTEM） |
| **PAYLOAD** | $N$ Byte | 任意データ | 制御指令、トルク値、センサ値、ステータスフラグ等 |
| **CRC-16** | 2 Byte | 計算値 | `LEN` 〜 `PAYLOAD` 末尾までを対象とする誤り検出コード |

---

### 4.3 マイコン（ATtiny1616）における実装防御機構

1. **LEN の不正値ガード（バッファオーバーラン防御）**
   受信した `LEN` が許容上限（64バイト）を超えていた場合、直ちに受信を中断（`abort`）してバスを沈黙させます。
   ```c
   #define MAX_PAYLOAD_SIZE (64)

   void on_frame_len_received(uint8_t len) {
       if (len > MAX_PAYLOAD_SIZE) {
           abort_reception_and_silence();
       } else {
           expected_remaining_bytes = len + 2; // PAYLOAD(len) + CRC16(2)
       }
   }
   ```
2. **CRC-16-CCITT による誤り検出**
   多項式 $x^{16} + x^{12} + x^5 + 1$（初期値: `0xFFFF`）を採用。長尺データやインバータ等の産業ノイズ下でも誤検出見逃し率を極限まで低減します。

---

## 5. 決定論的バトンリレーとスケジューリングモデル (Deterministic Relay & Scheduling)

### 5.1 自律分散 Pub/Sub リレーフロー

```text
============================== RS-485 共有バス ==============================
     ▲                              ▲                              ▲
     │ (1) Publish & Pass           │ (2) Publish & Pass           │ (3) Publish & Pass
     │   [Sender: 0x01]             │   [Sender: 0x02]             │   [Sender: 0x03]
     │   [Target: 0x02]             │   [Target: 0x03]             │   [Target: 0x01]
+----+-----------------------+ +----+-----------------------+ +----+-----------------------+
|    Node A (Publisher 0x01) | |    Node B (Publisher 0x02) | |    Node C (Publisher 0x03) |
|                            | |                            | |                            |
| 購読: 0x02, 0x03           | | 購読: 0x01                 | | 購読: 0x01, 0x02           |
| (Common Subscriber)        | | (Common Subscriber)        | | (Common Subscriber)        |
+----------------------------+ +----------------------------+ +----------------------------+
```

1. **バトンの受領と発話（Unique Publisher）:**
   受信パケットの `TARGET_ID` が自身の所有するバトンIDと一致した場合、直ちに次バトンを指定して送信します。
2. **共有バスでの全員購読（Common Subscriber）:**
   全ノードが受信し、`SENDER_ID`（トピック）を参照して必要なデータを自機の Rx Mailbox へ格納します。
3. **全員同期のタイマーリセット:**
   CRC 一致フレームが流れる限り、全ノードの内蔵生存監視タイマーが一斉にリセットされます。

---

### 5.2 非対称スケジューリング（Multi-rate Scheduling / 1デバイス複数ID）

```text
【ユースケース: 高速モーション制御 (Node A) と 低速センサ (Node B, C)】
・Node A (高速制御): バトン 0x01, 0x03 を所有（1周に2回発話 -> 1kHz）
・Node B (中速モータ): バトン 0x02 を所有（1周に1回発話 -> 500Hz）
・Node C (低速センサ): バトン 0x04 を所有（1周に1回発話 -> 500Hz）

リレー順序（1サイクル）:
[0x01 (Node A)] ──> [0x02 (Node B)] ──> [0x03 (Node A)] ──> [0x04 (Node C)] ──> [0x01 (Node A)] ...
```

#### ノード内ルーティングテーブル（`SlotRoute_t`）
```c
typedef struct {
    uint8_t my_slot_id;      ///< 自身が担当する論理バトンID (SENDER_ID)
    uint8_t next_target_id;  ///< 次にバトンを渡す宛先バトンID (TARGET_ID)
    uint8_t payload_len;     ///< このスロットで送出するデータ長
} SlotRoute_t;

const SlotRoute_t nodeA_routes[] = {
    { .my_slot_id = 0x01, .next_target_id = 0x02, .payload_len = 8  }, // 高速制御 (8B)
    { .my_slot_id = 0x03, .next_target_id = 0x04, .payload_len = 32 }  // 診断ログ (32B)
};
```

---

### 5.3 最悪実行時間（WCET）およびサイクルタイム算定モデル

ネットワーク全体の1周の総論理スロット数を $M$、スロット $k$ のペイロード長を $L(k)$、ボーレートを $B$ [bps]、1バイト送信時間を $T_{\text{byte}} = 10/B$、物理同期ヘッダ時間を $T_{\text{header}}$、送受信切替ギャップを $T_{\text{gap}}$ と置いたとき、システム全体のサイクルタイム $T_{\text{cycle}}$ は以下で一意確定します。

$$T_{\text{cycle}} = \sum_{k=1}^{M} \left( T_{\text{header}} + (3 + L(k) + 2) \times T_{\text{byte}} + T_{\text{gap}} \right)$$

---

## 6. メディエーション（調停）とシステム運用シーケンス (Mediation & System Sequences)

### 6.1 パッシブ・フェイルセーフ（断線・ノード故障時の自律心中）

```text
[Node A (Slot: 1)] ──(Target: 2)──> [Node B (Slot: 2 / 故障沈黙)]
                                         │ (バトン落下 = バス無音)
                                         ▼
                              【全ノード タイマー満了】
                                         │
                                         ▼
                         【全ノード 同期して安全停止 (STO)】
```

---

### 6.2 起動時 Ping 点呼シーケンス (Pre-operational Health Check)

本番リレー開始前に、Mediator（SYSTEM: `0xFF`）が全ノードの接続・健全性を 1 台ずつ確認するシーケンスです。

```text
SYSTEM (0xFF) ──(Ping: Target 0x01, Sender 0xFF)──> Node 1 (Pong: Target 0xFF, Sender 0x01) ──> SYSTEM (0xFF)
SYSTEM (0xFF) ──(Ping: Target 0x02, Sender 0xFF)──> Node 2 (Pong: Target 0xFF, Sender 0x02) ──> SYSTEM (0xFF)
SYSTEM (0xFF) ──(Ping: Target 0x03, Sender 0xFF)──> Node 3 (Pong: Target 0xFF, Sender 0x03) ──> SYSTEM (0xFF)
             │
             ▼ 【全ノード健全確認完了】
SYSTEM (0xFF) ──(点火フレーム: Target 0x01, Sender 0xFF)──> 【本番自律リレー開始 (0x01 -> 0x02 -> ...)】
```

* **効果:** 起動遅延や接触不良のノードがあっても、本番リレーの誤心中（STO）を引き起こすことなく安全に検出・特定できます。

---

### 6.3 稼働中インターリーブ診断スロット (Interleaved Diagnostic Slot)

通常リレーリングに 1 スロットだけ SYSTEM スロット（`0xFF`）を組み込む運用です。

```text
[Cycle 1]: 0x01 ──> 0x02 ──> [0xFF (SYSTEM: Ping 0x01 診断要求)] ──> 0x01 ──> 0x02 ...
[Cycle 2]: 0x01 ──> 0x02 ──> [0xFF (SYSTEM: Ping 0x02 診断要求)] ──> 0x01 ──> 0x02 ...
```

* 制御ジッターをゼロに拘束したまま、バックグラウンドでノードごとの詳細ログや温度推移を収集可能です。

---

### 6.4 意図的衝突（Forced Collision）と再点火フレーム (Re-Ignition)

* **強制停止:** Mediator がバスへ規定長以上のドミナント信号（Breakパルス）を強制送出し、全ノードを安全停止（STO）へ移行させます。
* **再点火:** バスの静寂を確認後、Mediator が `SENDER_ID = 0xFF`、`TARGET_ID = 0x01`（リング内の有効なバトン）のフレームを打ち込んでリレーを再開します。

---

## 7. 再送プラグイン (Retry Plugin) アーキテクチャ

```c
#include <stdint.h>
#include <stdbool.h>

#define DROP_ID_SYSTEM   (0xFF)  ///< SYSTEM / Mediator 専用スロットID
#define DROP_ID_ALL      (0xFF)  ///< 全バトン一括指定用ワイルドカードID

typedef enum {
    DROP_TRIGGER_NONE       = 0x00,
    DROP_TRIGGER_CRC_ERROR  = 0x01,  ///< CRC不一致によるパケット破損
    DROP_TRIGGER_TIMEOUT    = 0x02,  ///< 規定時間内に次のフレームが来ない（無音途絶）
    DROP_TRIGGER_BOTH       = 0x03   ///< CRC破損・無音途絶の双方を対象とする
} DropRetryTrigger_t;

typedef enum {
    DROP_ACTION_DROP_AND_STOP = 0,   ///< 再送を行わず安全停止状態へ誘導（受動的STO）
    DROP_ACTION_RETRY                ///< 再点火フレームを送出し、バトンを再始動させる
} DropAction_t;

typedef struct {
    uint8_t            target_baton_id; ///< 対象バトンID（個別ID または DROP_ID_ALL）
    DropRetryTrigger_t trigger;         ///< 監視対象トリガー
    uint8_t            max_retries;     ///< 最大リトライ許容回数（0 で即時STO）
    uint16_t           timeout_us;      ///< 無音監視タイムアウト時間（マイクロ秒）
    uint16_t           cooldown_us;     ///< 再点火フレーム送出前のバス安定化待機時間
} DropRetryPolicy_t;

DropAction_t drop_retry_evaluate(
    const DropRetryPolicy_t *policies,
    uint8_t policy_count,
    uint8_t failed_baton_id,
    DropRetryTrigger_t trigger,
    uint8_t *retry_counter
);
```

---

## 8. 比較優位性マトリクスと規格適合性 (Comparative Analysis & Compliance)

### 8.1 従来規格および PAL Network との比較

| 評価軸 | LIN | CAN / CAN-FD | PAL Network | **DROP-Bus** |
| :--- | :---: | :---: | :---: | :---: |
| **アーキテクチャ** | Master/Slave | Multi-Master CSMA/CD | **集中型 Polling Pub/Sub** | **自律分散 Baton-Relay Pub/Sub** |
| **マスター依存性** | 常時必須 (SPOF) | 不要 | 常時必須 (Master Broker) | **平常時不要 (SYSTEM 0xFF オプション)** |
| **Pub/Sub モデル** | なし | メッセージID | Topic-based Pub/Sub | **Baton-based Pub/Sub** |
| **ID空間** | 6-bit (0〜63) | 11/29-bit | 6-bit Topic ID | **8-bit (0x01〜0xFE: 254スロット, 0xFF: SYSTEM)** |
| **クロック要件** | 水晶レス (LINAUTO) | 高精度水晶必須 | 水晶レス (LINAUTO) | **完全水晶レス (LINAUTO)** |
| **通信ジッター** | 小 (ポーリング固定) | 大 (優先度・再送変動) | 小 (Master スケジュール) | **極小 (数式で拘束可能)** |
| **通信効率** | 低 (ヘッダ過多) | 中 (調停オーバーヘッド) | 中 (Poll/Response 2往復) | **最高 (純粋リレー 1往復ゼロ)** |
| **最大データ長** | 8 Byte | 8 / 64 Byte | 8 Byte | **最大 64 Byte** |
| **異常時動作** | マスター再送依存 | 自動再送で遅延拡大 | マスタースキップ・再送 | **受動的即時停止 (心中STO)** |
| **末端ノード単価** | 低 | 中〜高 (CANコントローラ) | 極低 (ATtiny1616 単一SKU) | **極低 (ATtiny1616 単一SKU)** |

---

### 8.2 機能安全規格（IEC 61508 / ISO 13849）適合への説明性

1. **ブラックボックスの排除**
   プロトコルスタックが裏で勝手な再送やバックオフを行わないため、安全機能の最悪応答時間（WCET）が明瞭に算出できます。
2. **単一故障時の安全側動作（Fail-Safe State）**
   断線、ノード電源喪失、CRC破損などのあらゆる単一故障において、プロトコルは「バス無音 $\rightarrow$ 全ノードタイマー満了 $\rightarrow$ ハードウェアSTO」へと一意に収束します。

---

## 9. 実装ガイドラインとロードマップ (Implementation Guidelines)

### 9.1 ADX Core-D（ATtiny1616）リファレンス実装
* **USART0 (LINAUTO モード):** `PA1` (TXD), `PA2` (RXD)
* **RS-485 方向制御 (DE/RE):** `PA4` / `PA7`（送受信切り替え）
* **生存監視タイマー:** 16-bit タイマー `TCB0`（ワンショットまたは周期モード）
* **Double Buffer Mailbox:** 送信2面（Tx Mailbox）＋ 購読トピック受信2面（Rx Mailbox）
* **SYSTEM 0xFF ハンドラ:** `TARGET_ID == 0xFF` 受信時の Pong 返答または調停処理

---

### 10. まとめ

DROP-Bus は、**8-bit フル空間（254個のユーザー論理スロット ＋ `0xFF` SYSTEMスロット）** を備え、自律分散バトンリレーの高速リアルタイム制御と、SYSTEMスロットによる安全な起動時点呼（Ping）・保守調停を完璧に両立させた決定版の産業用フィールドネットワークです。
