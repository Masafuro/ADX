# 製品コンセプト

## 製品名
- ADX CORE-S

## 製品情報 (Origin Product)
- hardware/CORE-S
  - rev4以降

## コンセプト
- 音響機器のような手軽さで
- 利用、敷設可能な
- オープンソースの
- プログラマブルコントローラー

## 主機能要素
- Arduino like Development
  - with megaTinyCore
  - Arduino IDEやArduino CLIによるコンパイル
  - 書き込みは別で専用アプリケーションやスクリプトを提供
- Enclosure Friendly
  - 基板の4隅にM3×4、ワッシャーなどのためのΦ10クリアランスを持つ
  - スロットイン用のノッチ2か所、スロットイン用の側面設置禁止エリア
  - 前面長辺は任意の端子を与えるI/F辺として指定
  - 後面長辺はIDC 2×10p ボックスヘッダによりGPIOを拡張可能
- Field Network Ready
  - RS-485 OTW対応
  - 電源入力12V~24V
  - チェーン可能なSpring Clamp Terminalを2系統
  - 電源入力及びRS-485は絶縁

## Arduinoなど普通のマイコンボードと異なる点
- 書き込みにRS-485 USBシリアル変換器が **必須**
  - 絶縁版を推奨
  - WeAct USB2RS485など
- USBやDCジャックは **非搭載**
- UNOフォームファクタには **非対応**, 専用のADXフォームファクタとなる。

## 想定価格
- 35ドル前後