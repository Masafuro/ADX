# ADX
ADX: Advanced Devices eXtended's project documents.
- ADX: Advanced Devices eXtended
- It's a form factor for Industrial ATX and Next Gen Arduino.
- 1st Goal is to start Crowd Supply.
## 1. 現状
### Product Data
#### ADX Core V0
- [Project Data](dev/ADX_Core_v0/Manufacture/20260430_1154)
![](dev/ADX_Core_v0/Manufacture/20260430_1154/3dview_front.png)


## ADX Core V0 暫定製品仕様書
### 1. 一般性能仕様 (General Specifications)
| 項目 | 仕様 | 備考 |
|---|---|---|
| **制御方式** | ストアドプログラム方式 | (ATtiny1616-MNR) |
| **基本演算速度** | 16 MHz | (推奨設定値) |
| **ロジック電圧** | 5.0 V |  |
| **メモリ容量** | Flash: 16 KB / SRAM: 2 KB | EEPROM: 256 B |
| **動作周囲温度** | -40 ℃ ～ +105 ℃ | (結露なきこと) |
### 2. 電源仕様 (Power Supply Specifications)
| 項目 | 仕様 | 備考 |
|---|---|---|
| **定格入力電圧** | DC 12 V / 24 V / 48 V |  |
| **入力電圧許容範囲** | **DC 7.0 V ～ 48.0 V** |  |
| **電源接続方式** | 2ピン プッシュイン式ターミナル | ロックレバー付 |
| **過電流保護** | **表面実装型チップヒューズ搭載** | 使い捨てタイプ |
| **逆接保護** | ダイオード極性保護回路搭載 |  |
| **サージ防護** | **TVSダイオード搭載** |  |
### 3. 通信仕様 (Communication Specifications)
| 項目 | 仕様 | 備考 |
|---|---|---|
| **通信規格** | **RS-485** × 1 系統 |  |
| **接続端子** | 2ピン プッシュイン式ターミナル | RS-485 (A, B) |
| **サージ防護** | **TVSダイオード搭載** |  |
| **プログラミング** | UPDI (3ピンソケット) |  |
### 4. 入出力仕様 (I/O Specifications)
| 項目 | 仕様 | 備考 |
|---|---|---|
| **GPIO点数** | **20 点** | 5V CMOS / TTLレベル |
| **コネクタ形式** | 2×10ピン IDCボックスヘッダ | ロック付 / ライトアングル |
### 5. 外形・構造仕様 (Mechanical Specifications)
| 項目 | 仕様 | 備考 |
|---|---|---|
| **外形寸法** | 85.0 mm (W) × 60.0 mm (H) |  |
| **取付穴位置** | 四隅 4箇所 (M3ネジ対応) |  |
| **取付穴ランド** | φ8.0 mm | ワッシャー接触用 |
| **基板加工** | 四隅 C3 面取り |  |

#### ADX IDCtoPinheader Adapter
- [overview](dev/ADX_IDCtoPinheader_adapter/20260429_1705/overview.md)
![](dev/ADX_IDCtoPinheader_adapter/20260429_1705/3D_front.png)

## 2. 進捗ログ
- [Project_Snapshot.md](Project_Snapshot.md)
	- [History](https://github.dev/Masafuro/ADX/blob/main/Project_Snapshot.md)
- [Main Site: ADX platform](https://adxplatform.com/)
	- [Blog](https://dev-blog.adxplatform.com/)
