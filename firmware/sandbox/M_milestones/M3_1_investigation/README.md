# Milestone 3-1 (M3-1): Flash 書き込み後ハードウェア挙動 調査診断スイート

## 概要
本ディレクトリは、Milestone 3（Flash 1ページ物理書き込み）において発生した「TEST 3完了後の TEST 4 (Readback) / TEST 5 (CRC Check) の不成立」の原因を、**目視に頼らずコンソールの文字出力・生パケット解析のみで完全に解明・白黒つける**ための調査診断スイートです。

---

## 調査の狙いと仮説検証

```
                    [ 課題: TEST 4, 5 が不成立 ]
                                  │
      ┌───────────────────────────┴───────────────────────────┐
      ▼                                                       ▼
【仮説 A: ハードウェア過渡状態説】               【仮説 B: プロトコル・新コマンド齟齬説】
NVMCTRL書込後にマイコンがストール/異常            マイコンは生存しているが、Read/CRC コマンドの
状態となり以降の通信を受け付けない。               送受信フォーマットやCRC計算で弾かれている。
```

---

## 収録ファイル一覧

| ファイル名 | 役割 |
|---|---|
| [`m3_1_diag_bench.py`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/sandbox/M_milestones/M3_1_investigation/m3_1_diag_bench.py) | **コンソール診断用ホストスクリプト**（生パケット全HEXダンプ、生存確認、レジスタテレメトリ取得） |
| [`m3_1_diag.c`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/sandbox/M_milestones/M3_1_investigation/m3_1_diag.c) | **ハードウェア診断対応マイコンファームウェア**（NVMCTRLクリーンアップ、RSTFR/STATUSテレメトリ応答） |
| `m3_1_diag.hex` | コンパイル済みバイナリ（1,896 Bytes） |
| `Makefile` | ビルド設定ファイル |

---

## 実行手順（コンソールでの調査手順）

### ステップ 1: 現在のファームウェアのまま生パケットをダンプ（非破壊）
マイコンを書き換えずに、現在の状態で「マイコンから何が返ってきているのか」を全バイト可視化します。
```powershell
python m3_1_diag_bench.py --port COM22 --mode inspect --page 20
```
* **得られる情報**:
  * Chunk 0〜3 の要求に対するマイコンの生の 32 バイト返信 HEX。
  * 受信 CRC と期待 CRC の照合結果。
  * マイコンが何を返してパースが弾かれていたかが即座にコンソールに表示されます。

### ステップ 2: Flash 書き込み直後の即時生存テスト（Liveness Probe）
Page 20 に物理書き込みを行った「直後（0ms、20ms後、100ms後）」に Ping を打って、マイコンの生存をテストします。
```powershell
python m3_1_diag_bench.py --port COM22 --mode survival --page 20
```
* **得られる情報**:
  * Flash 書込直後にマイコンが生きているか、ストールしているかが 100% 判定できます。

### ステップ 3: ハードウェア診断ファームウェアによるレジスタ解析（任意）
`m3_1_diag.hex` を Core-D に書き込むことで、マイコン内部の `RSTCTRL.RSTFR`（リセット要因）や `NVMCTRL.STATUS`（Flashビジー状態）をパケットで取得できます。
```powershell
# ファームウェア書き込み (SerialUPDI: COM20)
pymcuprog write -d attiny1616 -t uart -u COM20 -c 115200 -f m3_1_diag.hex --erase

# テレメトリ取得
python m3_1_diag_bench.py --port COM22 --mode telemetry
```
