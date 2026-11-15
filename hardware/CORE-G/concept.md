# ADX CORE-G 概念

## 概要
まず、ベースとなったCORE-UからUSB TYPE-Cを1本化
その前提は
https://www.waveshare.com/usb-c-power-splitter.htm?srsltid=AYCX9vJF_AsY8j1J9LOU7XVlL4gH8qielJHkejbgLHGl_X_LdiPDSDj3
型のUSB TYPE-C 電源通信スプリッターの発見による。
すなわち、取り扱う電流が、500mAクラスで十分であれば、USB TYPE-C1本で開発できるし、もし2A程度を扱いたい場合は、上記のスプリッターを使うことで、給電できるはずだ。
そして、I/O Edge側の拡張端子をGROVEに統一。ブザーはフルカラーLEDを廃止して、余ったGPIOをできるだけGROVEポートにすることに特化した構想。また、これにより、GROVE端子はM5Unitに対応した5V, GND, GPIO×2の系統として明確になった。

## 実装費用概算
JLCPCB, PCBA込, 弊社宛送料込み
- $504.75 @ 120pcs
- 製造原価約 $4.21 @ 1pcs
- 想定販売価格 $17~20程度

## 懸念点
Arduino UNOの13番pinのようなLEDが欲しいが、すでにピンは残っていないかもしれない。しかし、Arduino UNOでもシールド側に13番pinは引き出しているので、どこかに重複してLEDを設置可能なピンはあるかもしれない。