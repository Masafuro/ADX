# ADXファミリー命名規則

## 結論
`ADX CARD | 製品識別名 | [グレード]`
`ADX CORE | 製品識別名 | [グレード]`

グレードは **無印（標準・卓上開発／プロトタイピング向け）** と **`PRO`（業務用・現場常設／定格試験保証グレード）** の2区分とする。
※ 詳細は [ハードウェア命名規則・グレード分類基準](../docs/ja/hardware_naming_rules.md) を参照。

### 例
**ADX CARD**（機能・仕様を表す名称 ＋ 必要に応じて `PRO`）
- `ADX CARD-Prototyping`
- `ADX CARD-Power AAA 2S`
- `ADX CARD-Power DC12V24V ISO PRO`
- `ADX CARD-DC12V/24V I/O PRO`
- `ADX CARD-Ethernet PRO`
- `ADX CARD-DC-48V ISO PRO`

**ADX CORE**（モデル・プラットフォームを表す名称 ＋ 必要に応じて `PRO`）
- `ADX CORE-G`（無印：USB Type-C 1本化・Grove/M5Unit特化プロトタイピング機）
- `ADX CORE-U`（無印：デュアル Type-C 標準USB開発機）
- `ADX CORE-A PRO` / `ADX CORE-A24 PRO`（PRO：絶縁RS-485・高堅牢端子台・定格試験保証の現場機）
- `ADX CORE-RP` / `ADX CORE-ESP32`
