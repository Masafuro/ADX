<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# Milestone 0 検証レポート: Windows 11 実行環境セットアップ & 導通確認

## 1. 概要
- **目的**: 実機操作を行う Windows 11 PC において、Core-D（SerialUPDI）および USB RS-485 ドングルが正常に認識され、マイコン（ATtiny1616）と電気的・論理的に導通できるベースラインを確立する。
- **実施日**: 2026-09-27
- **検証環境**:
  - ホスト PC: Windows 11
  - ターゲットボード: ADX Core-D（初版基板、MCU: ATtiny1616-MNR）
  - RS-485 ドングル: 2Mbps 対応 USB RS-485 変換アダプター

---

## 2. 実行手順 & チェックリスト

### ① Python 環境 & ライブラリ導入
Windows 11 の PowerShell またはコマンドプロンプトを開き、以下を実行してツールを準備します。

```powershell
# Python バージョン確認 (3.8 以上推奨)
python --version

# 必要ツールのインストール
pip install pymcuprog pyserial
```

- [x] Python が正常に動作する
- [x] `pymcuprog` および `pyserial` のインストール完了

---

### ② 仮想 COM ポートの特定
1. **Core-D（CH342K）の接続**:
   - Core-D を USB Type-C ケーブルで Windows 11 PC に接続。
   - デバイスマネージャーの「ポート (COM と LPT)」を確認。
   - **確認された COM ポート番号**: `COM20`, `COM21` (SerialUPDI 側はたぶんCOM20)
2. **USB RS-485 ドングルの接続**:
   - ドングルを PC の USB ポートに接続。
   - デバイスマネージャーで新たな COM ポートを確認。
   - **確認された COM ポート番号**: `COM19` (RS-485 側)

- [x] Core-D の COM ポート（CH342K）が特定できた
- [x] USB RS-485 ドングルの COM ポートが特定できた

---

### ③ SerialUPDI 導通確認 (Ping)
Windows 11 の PowerShell から、Core-D の COM ポートを指定して以下のコマンドを実行します：

```powershell
# 例: Core-D が COM3 の場合
pymcuprog ping -d attiny1616 -t uart -u COM3
```

#### 期待される出力ログ例:
```text
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Done.
```
※ `1E9421` は ATtiny1616 の固有デバイス ID（Signature）です。

- [x] `pymcuprog ping` がエラーなく成功した
- [x] デバイス ID `1E9421`（または ATtiny1616 応答）が確認できた

---

## 3. 実際の実行ログ記録欄

（※Windows 11 のターミナル出力をここに貼り付けて記録してください）

```text
PS C:\Users\User> pymcuprog ping -d attiny1616 -t uart -u COM20
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Done.```

---

## 4. 合否判定（Exit Criteria）

| 判定項目 | 合格基準 | 結果 |
| :--- | :--- | :---: |
| **Python ツールチェーン** | `pymcuprog` コマンドが認識されること | [x] PASS |
| **COM ポート認識** | Core-D と RS-485 ドングルのポート番号が一意に特定できていること | [x] PASS |
| **UPDI 導通** | `pymcuprog ping` で ATtiny1616 の Device ID が読めること | [x] PASS |

**総合判定**: 【 PASS : 2026/09/27 10:34 】

---

## 5. 次のマイルストーンへの引き継ぎ
Milestone 0 の PASS を確認次第、**Milestone 1（Ubuntu 側での 512B ブートローダーのビルド & サイズ検証）** へ移行し、Windows 11 側へ投入する `.hex` バイナリを生成します。
