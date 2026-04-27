# Project Snapshot

## 0. プロジェクト
- ADX: Advanced Devices eXtended
- It's a form factor for Industrial ATX and Next Gen Arduino.
- 1st Goal is to start Crowd Supply.

## 1. 現状
- ADX Core V0
    - 回路基本設計を完了。寸法記入を完了。
      - via 足りてない問題がある。
- ADX Core V0 レビュー
    - [ ] BOM
        - [ ] 温度域　`-40℃ ~ 105℃`
        - [ ] 在庫数　`5,000以上`
    - [ ] Netlist
        - [ ] 周辺回路
            - [ ] ATtiny1616-MNR
            - [ ] LMR36506RRPER
            - [ ] THVD2410DR
        - [ ] RCL配置
    - [ ] NetAnalytics
        - [ ] 配線長
        - [ ] 配線数
        - [ ] via数
        - [ ] ばらつき
    - [ ] Pickandplace
        - [ ] 部品間距離
        - [ ] ターミナル位置
        - [ ] ネジ位置
- ADX Core V0 試験
    - 試験計画を立案した。
      - [試験概要](dev/ADX_Core_v0/test/0_test_overview.md)
    - 購入するもの
      - 顕微鏡、USB-UPDI、USB-RS485
- ADX DEBUG BOARD
    - 寸法がまだ。
- ADX LED Tape Drive CARD
    - 基本計画を立案した。
    - LEDテープ型で5Wくらいまでは行けそうな感じ。
    - [LEDテープドライブ拡張カードの検討](https://memo.adxplatform.com/note/PL4%EF%BC%88Power%20over%20LIN%20based%20RS485%EF%BC%89LED%E3%83%86%E3%83%BC%E3%83%97%E7%94%A8%E3%83%89%E3%83%A9%E3%82%A4%E3%83%96%E3%83%9C%E3%83%BC%E3%83%89%E6%A4%9C%E8%A8%8E%E8%B3%87%E6%96%99)
- Github
    - パーマリンクがスマホだとだめ。
    - 相対リンクに引き直しが必要。
- Hackster.io
    - 古いまま。
- Hackaday.io
    - どこかでログを追加していく必要がある。
- LinkedIn
    - 草稿を作成した。
- ADXplatform トップページ
    - サイト間リンクの再検討が必要そう。
- セキュリティ
    - tailscaleの掃除
 
### 1.1 買い物リスト
- 顕微鏡
- USB-UPDI
- USB-RS485
- 20pin IDC リボンケーブル

## 2. 違和感
- 今のところ見られない

## 3. 一時完了

## 4. 仮説リスト

| 提案時期 | 仮説 | 状態 | 反証条件 |
|----|------|------|-----|
| 2026/04/23 | 顧客やファンとはブログをポストしたり、Hackaday.ioをポストしたくらいでは出会えないと思う。 | 検証中 | うっかりなんらかの出会いがある。 |


## 5. 検証済リスト

| 検証時期 | 仮説 | 検証結果 | 棄却／確定 | 得られた学び |
|----|------|----------|--------------|----------------|
| 2026/03/22 | PCBAできること。 | ATtiny1616-MNRの試作に成功した | 確定 | JLCPCBやAIの回路レビュー能力は十分に高い可能性がある。ただし、人間の側が間違う。（UI表記を見間違って、ダイオードの逆接した。） |
| 2026/03/22 | Arduino IDEで開発できるボードを制作できること。 | ATtiny1616-MNRの試作でmegaTinyCoreを使い、スケッチを書き込みLチカできた。 | 確定 | 書き込み速度を落とす必要があった。高速通信などの部分で回路設計に十分なスキルが伴っていない可能性がある。 |
| 2026/04/24 | プロジェクトの総体的な管理にGit-HT方式による単純テキスト及び差分管理方式が利用できるのではないか？ | スマホも含めて便利に利用できる事がわかった。 | 確定 | ぶっちゃけあんまり振り返りはしていないし、いざとなればEditerでhistoryも見える。過去の情報はsnapshotからはどんどん消えていくのがよいようだ。 |

## 6. ノウハウ
- Git-HT
    - github.devに移動し、WEB Editorを開き、ファイルを開いてから右クリックで、TIMELINEを確認できる。
    - スマホのchromeから編集できる。
- 回路設計
    - ターミナルの位置を一番最初に筐体とあわせてきっちり座標ぎめしておかないと事故になる。

## 7. 完了したもの
