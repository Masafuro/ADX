# ADX CORE-A5

ADX CORE CARD
with ATtiny1616 and 5V power supply

## 概要
CORE-AからCORE-Rを経て、なんやかんやあってCORE-Aに戻りつつ、様々な仕様変更を経てCORE-A5が策定された。

## 特徴
- 耐熱選定: Indsutrial -40℃~105℃以上
- RS-485: 絶縁500kbps, Spring Clamp Terminal 2P*2
- 5VIN: Spring Clamp Terminal 2P, 短絡保護, 過電流保護, 逆流防止
- 拡張端子: ADX IDC 2*10P ( 5VSYS(4), GPIO 3V3トレラント )
- MCU: ATtiny1616-MNR

## スケッチ書き込み
- UPDI: UPDI書き込み器が必要
- RS-485: LN-485系（検討中）通常USB RS-485ドングル、PCからの書き込みを想定
  - RS-485ブートローダー: なんらかの特定のコマンドによってフィールドネットワーク内のADX COREをすべてリセットを許容、ブートローダーモードで起動。リセットコマンドはRS-485からCCLで処理する（詳細未定）。ブートローダー起動後、SIGROW指定で特定のユニットへスケッチを書きこみ

## その他
- LEDは車載耐熱3色LEDを利用し、Gを電源からONに、R,BはMCUから制御
  - Power ON: Green
  - MCUの制御で、G+R, G+B, G+R+Bなどの表現が可能

## 想定価格
- 想定販売価格: 22ドル
- 荒見積り状況
  - 10 Panels, 100 Single Pieces
  - Green, 1.6 Thickness, OSP
  - 弊社宛送料込: 552.86ドル
