# Project Snapshot

## 0. プロジェクト
- ADX: Advanced Devices eXtended
- It's a form factor for Industrial ATX and Next Gen Arduino.
- 1st Goal is to start Crowd Supply.

## 1. 現状
- ADX Core V0
    - 回路基本設計を完了。寸法、試験項目を記入完了。
    - Notebooklmのレビューがまだ。
- ADX Core V0 試験
    - 試験計画を立案した。
      - [試験概要](dev/ADX_Core_v0/test/0_test_overview.md)
    - 購入するもの
      - 顕微鏡、USB-UPDI、USB-RS485
- ADX DEBUG BOARD
    - 発注可能と思われる。
- ADX LED Tape Drive CARD
    - 基本計画を立案した。
    - LEDテープ型で5Wくらいまでは行けそうな感じ。
    - [LEDテープドライブ拡張カードの検討](https://memo.adxplatform.com/note/PL4%EF%BC%88Power%20over%20LIN%20based%20RS485%EF%BC%89LED%E3%83%86%E3%83%BC%E3%83%97%E7%94%A8%E3%83%89%E3%83%A9%E3%82%A4%E3%83%96%E3%83%9C%E3%83%BC%E3%83%89%E6%A4%9C%E8%A8%8E%E8%B3%87%E6%96%99)
- Github
    - パーマリンクがスマホだとだめ。
    - 相対リンクに引き直しが必要。
- Hackster.io
    - 古いまま。
    - そろそろ少し進捗更新した方がいいかも？
- Hackaday.io
    - ようやくプロジェクトページを作成できた。
    - どこかでログを追加していく必要がある。
    - たぶん、ADX Core V0 の発注時？
- LinkedIn
    - やっとページを作れた。
    - Profileページの内容はまだ記入できてない。
- ADXplatform トップページ
    - サイト間リンクの再検討が必要そう。
- セキュリティ
    - tailscaleの掃除
## 2. 違和感
- 今のところ見られない

## 3. 一時完了

## 4. 仮説リスト

| 提案時期 | 仮説 | 状態 | 反証条件 |
|----|------|------|-----|
| 2026/04/23 | 顧客やファンとはブログをポストしたり、Hackaday.ioをポストしたくらいでは出会えないと思う。 | 検証中 | うっかりなんらかの出会いがある。 |
| 2026/04/24 | プロジェクトの総体的な管理にGit-HT方式による単純テキスト及び差分管理方式が利用できるのではないか？ | 検証中 | プロジェクト全体の違和感や振り返りなどができない。 |

## 5. 検証済リスト

| 検証時期 | 仮説 | 検証結果 | 棄却／確定 | 得られた学び |
|----|------|----------|--------------|----------------|
| 2026/03/22 | PCBAできること。 | ATtiny1616-MNRの試作に成功した | 確定 | JLCPCBやAIの回路レビュー能力は十分に高い可能性がある。ただし、人間の側が間違う。（UI表記を見間違って、ダイオードの逆接した。） |
| 2026/03/22 | Arduino IDEで開発できるボードを制作できること。 | ATtiny1616-MNRの試作でmegaTinyCoreを使い、スケッチを書き込みLチカできた。 | 確定 | 書き込み速度を落とす必要があった。高速通信などの部分で回路設計に十分なスキルが伴っていない可能性がある。 |

## 6. ノウハウ
- Git-HT
    - github.devに移動し、WEB Editorを開き、ファイルを開いてから右クリックで、TIMELINEを確認できる。
    - スマホのchromeから編集できる。

## 7. 完了したもの
