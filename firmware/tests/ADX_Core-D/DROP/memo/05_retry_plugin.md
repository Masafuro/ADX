
# 産業用フィールドネットワーク DROP-Bus 仕様書：再送プラグイン (Retry Plugin) アーキテクチャ

**DROP-Bus** (**D**eterministic **R**elay with **O**ptional-mediator & **P**assive-failsafe Bus) において、プロトコル・コア（Core）は一切の再送機能を持たない。再送はプロトコル層の内部処理として隠蔽されるべきではなく、時間軸（ジッター）と安全性を統制する**システム設計者の裁量下**に置かれるべき機能である。

本稿では、コアスタックの決定論性と軽量性を損なうことなく、現場要件に応じた柔軟な復旧制御を実現する**「再送プラグイン（Retry Plugin）」**のアーキテクチャおよびAPI仕様を規定する。

---

## 1. 再送プラグインの設計思想

1. **プロトコル・コアからの完全分離 (Decoupled Core)**
   * DROP-Bus Core は「バトンのリレー」と「通信途絶時の受動的停止（Safe Torque Off: STO）」のみを担う。
   * 単一SKUとして動作する末端ノード基板には再送プラグインをリンクせず、ファームウェアフットプリントを最小限（数十〜数百バイト水準）に維持する。
2. **決定論（タイムバジェット）の完全可視化**
   * 再送処理による最大遅延時間は、プラグインの設定テーブルから静的に算出可能とする。
   * 制御ループの最悪実行時間（WCET: Worst-Case Execution Time）が数学的に証明可能となり、機能安全規格（IEC 61508 / ISO 13849）への適合性を担保する。
3. **プラグインの配置自由度**
   * 独立した専用Mediatorユニットだけでなく、特定のノード（PLC直結ノードや局所区間ノード）に対しても任意に組み込み可能とする。

---

## 2. アーキテクチャ階層構造

```text
+-------------------------------------------------------------------+
|                        DROP-Bus Core                              |
|   - LINAUTO 物理同期 (Break + 0x55)                               |
|   - 固定長 / 最大長拘束型可変長リレー                              |
|   - ハードウェアタイマー監視 ＆ パッシブ・フェイルセーフ (STO)     |
|   - ゼロ再送・即時ドロップ                                        |
+-------------------------------------------------------------------+
                                  ▲
                       フック呼び出し (Hook API)
                                  │
+---------------------------------┴---------------------------------+
|                    Retry Plugin (任意モジュール)                  |
|   - 異常検知 (CRC破損 / 無音タイムアウト)                         |
|   - 適用対象フィルタリング (特定ノードID / 全ノード一律)          |
|   - 実行制御 (最大リトライ回数 / 許容時間枠 / クールダウン)       |
|   - 調停シグナル送出 (Break衝突による停止 / 再点火フレーム生成)   |
+-------------------------------------------------------------------+

```
## 3. データ構造とAPI仕様
### 3.1 再送トリガー条件とアクション
```c
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 再送判定の契機となるエラー要因
 */
typedef enum {
    DROP_TRIGGER_NONE       = 0x00,
    DROP_TRIGGER_CRC_ERROR  = 0x01,  ///< CRC不一致によるパケット破損
    DROP_TRIGGER_TIMEOUT    = 0x02,  ///< 規定時間内に次のフレームが来ない（無音途絶）
    DROP_TRIGGER_BOTH       = 0x03   ///< CRC破損・無音途絶の双方を対象とする
} DropRetryTrigger_t;

/**
 * @brief プラグインがコアへ指示する調停アクション
 */
typedef enum {
    DROP_ACTION_DROP_AND_STOP = 0,   ///< 再送を破棄し、バスを停止状態へ誘導（受動的STO）
    DROP_ACTION_RETRY                ///< 再点火パケットを送出し、バトンを再始動させる
} DropAction_t;

```
### 3.2 ポリシー定義構造体 (DropRetryPolicy_t)
システム設計者は、以下の設定構造体配列を定義することでネットワークの時間挙動を制御する。
```c
#define DROP_ID_ALL   (0xFF)  ///< 全ノード一括指定用ワイルドカードID

typedef struct {
    uint8_t            target_id;    ///< 対象ノードID（単一ID または DROP_ID_ALL）
    DropRetryTrigger_t trigger;      ///< 監視対象トリガー
    uint8_t            max_retries;  ///< 最大リトライ許容回数（0 で即時STO）
    uint16_t           timeout_us;   ///< 無音監視タイムアウト時間（マイクロ秒）
    uint16_t           cooldown_us;  ///< 再点火フレーム送出前のバス安定化待機時間
} DropRetryPolicy_t;

```
### 3.3 コールバックおよびフック関数
Mediatorロールを持つエンティティがパケット異常または無音を検知した際、本ハンドラが呼び出される。
```c
/**
 * @brief 再送ポリシー評価関数
 * 
 * @param[in]  policies            設計者が定義したポリシーテーブル
 * @param[in]  policy_count        ポリシー数
 * @param[in]  failed_target_id    バトンを受け取れなかった/返信しなかったノードID
 * @param[in]  trigger             検知された障害要因
 * @param[in,out] retry_counter    現在の累積リトライ回数カウンタへのポインタ
 * @return DropAction_t            RETRY（再送実行）または DROP_AND_STOP（安全停止）
 */
DropAction_t drop_retry_evaluate(
    const DropRetryPolicy_t *policies,
    uint8_t policy_count,
    uint8_t failed_target_id,
    DropRetryTrigger_t trigger,
    uint8_t *retry_counter
);

```
## 4. 適用構成例
### ケース1：過酷環境区間の局所リトライ（耐ノイズ性と決定論の両立）
スリップリングや可動ケーブルベア等を経由する特定ノード間（例: Node 0x04 宛て）のみ局所的に1回のリトライを許容し、他ノードは即時停止させる構成。
```c
const DropRetryPolicy_t local_resilient_policy[] = {
    // Node 0x04 宛て：CRCエラー発生時のみ1回だけ再送を試行（遅延枠: 500us）
    {
        .target_id   = 0x04,
        .trigger     = DROP_TRIGGER_CRC_ERROR,
        .max_retries = 1,
        .timeout_us  = 500,
        .cooldown_us = 50
    },
    // 全体デフォルト：いかなるエラーでも即時安全遮断（再送ゼロ）
    {
        .target_id   = DROP_ID_ALL,
        .trigger     = DROP_TRIGGER_BOTH,
        .max_retries = 0,
        .timeout_us  = 800,
        .cooldown_us = 0
    }
};

```
 * **最悪遅延の保証:** システム設計者は、最大遅延時間が通常周期に対して +550\,\mu\text{s} に収まることを数式で検証可能。
### ケース2：同期多軸ロボット（完全決定論・即時停止構成）
1フレームの欠落も制御破綻に直結するため、一切の再送を禁止し、100%の即時心中を強制する構成。
```c
const DropRetryPolicy_t strict_deterministic_policy[] = {
    {
        .target_id   = DROP_ID_ALL,
        .trigger     = DROP_TRIGGER_BOTH,
        .max_retries = 0,            // リトライ回数ゼロ
        .timeout_us  = 300,
        .cooldown_us = 0
    }
};

```
 * **効果:** ジッターを完全にゼロとし、通信途絶時は最短時間（300\,\mu\text{s}）で全軸STOへ倒す。
## 5. 再送プラグインがもたらす工学的価値
| 項目 | プロトコル内包型（CAN/LIN等） | DROP-Bus 再送プラグイン方式 |
|---|---|---|
| **ジッター制御** | ブラックボックス（予測困難） | **数式で100%可視化・拘束可能** |
| **末端ノード負荷** | 全ノードに再送ステートマシンが必要 | **末端ノードは0バイト追加（完全単一SKU維持）** |
| **非常停止の確実性** | 再送ループにより停止遅延の危険性 | **再送を停止・コリジョン注入で確実に全員心中** |
| **モジュール性** | 変更不可（規格に固定） | **用途に応じたテーブル定義の差し替えのみで完結** |
```
