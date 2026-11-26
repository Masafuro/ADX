# EasyEDA Agent 環境構築チェックリスト

[milestone_plan.md](milestone_plan.md) に沿って進めるための実作業チェックリストです。  
各マイルストーンの項目を上から順に確認し、問題があれば切り分けて対処します。

> **注意**: 接続用トークンや秘密鍵パスフレーズなどの機密情報は、このファイルや Git 管理下には記載しないでください。

---

## 環境メモ（作業しながら記入）

- **EasyEDA Pro バージョン**: `4.1.105.0339a6cd / International`（半オフラインモード有効）
- **EasyEDA Pro 実行ファイルパス**: `C:\Program Files\easyeda-pro\easyeda-pro.exe`
- **ローカルプロジェクト保存先**: `C:\Users\User\Documents\EasyEDA-Pro\projects`（M4 プロジェクト情報から確認済み）
- **Node.js バージョン（Windows / Ubuntu）**: `Windows: 未使用（不要） / Ubuntu: v20.20.2`
- **採用接続方式**: `[x] SSH リバーストンネル（推奨・AG-G3 経由）` / `[ ] Node.js フォアグラウンド中継`
- **使用ローカルポート**: `3030`（変更時はここを更新）

---

## M0: Windows 側への EasyEDA Pro デスクトップ版導入

- [x] [EasyEDA ダウンロードページ](https://easyeda.com/page/download) から **EasyEDA Pro デスクトップ版（International 版 / v4.1.70 以上）** を取得・インストールした
- [x] 既存の Web 版アカウントで EasyEDA Pro にログインできた
- [x] 半オフライン／ローカル保存モードの設定と、`Documents\EasyEDA-Pro\projects` フォルダの存在を確認した
- [x] コマンドプロンプト / PowerShell で `easyeda-pro --help` が実行でき、バージョン情報が表示された
  - *(パスが通っていない場合はインストール先フォルダを環境変数 `PATH` に追加)*
- [x] `easyeda-pro --mcp --help` を実行し、`--mcp http` で利用可能なオプション（`--port` / `--token` / ホスト指定可否など）を確認・記録した
- [x] EasyEDA Pro GUI を起動した状態で `easyeda-pro doctor` を実行し、`"ok": true` かつ `"connected": true` が返ることを確認した

---
### 開発者状況
2026/11/26 08:46

```
PS C:\Users\User> easyeda-pro doctor | Out-Host

{"ok":true,"value":{"endpoint":"EasyEDAPro740b1ecb","connected":true,"bridgeVersion":"4.1.105.0339a6cd","versionMatch":true,"resultChannel":true},"logs":[],"durationMs":107}

```

2026/11/26 08:48
```
PS C:\Users\User> easyeda-pro session list | Out-Host

{"ok":true,"value":[{"sessionId":"29c84e56-61d9-46b7-a429-6a3998366756","renderId":2,"status":"alive","headless":false,"origin":"user","path":null,"title":"JLCEDA Pro - V4.1.105"}],"logs":[],"durationMs":1}
PS C:\Users\User> easyeda-pro open | Out-Host

{"ok":true,"value":{"sessionId":"374efcd5-3462-4103-9f72-3385fcd916c7","renderId":3,"path":"","headless":false},"logs":[],"durationMs":2189}


```

2026/11/26 08:49

```
PS C:\Users\User> $S = "374efcd5-3462-4103-9f72-3385fcd916c7"
PS C:\Users\User> easyeda-pro invoke --session $S --ext-uuid eda --code 'return { isClient: eda.sys_Environment.isClient(), isHalfOfflineMode: eda.sys_Environment.isHalfOfflineMode(), version: eda.sys_Environment.getEditorCurrentVersion(), projectsPaths: await eda.sys_FileSystem.getProjectsPaths() };' | Out-Host

{"ok":true,"value":{"isClient":true,"isHalfOfflineMode":true,"version":"4.1.105","projectsPaths":[]},"logs":[],"durationMs":152}


PS C:\Users\User> [Console]::OutputEncoding = [System.Text.Encoding]::UTF8

PS C:\Users\User> easyeda-pro doc api --class-name DMT_Project | Out-Host

{"ok":true,"value":{"title":"文档树 / 工程管理类","visibility":"public","methods":[{"name":"openProject","comment":"打开工程","visibility":"public"},{"name":"createProject","comment":"创建工程","visibility":"beta"},{"name":"modifyProjectFriendlyName","comment":"修改工程友好名称"},{"name":"modifyProjectDescription","comment":"修改工程描述"},{"name":"modifyProjectCollaborationMode","comment":"修改工程协作模式"},{"name":"moveProject","comment":"移动工程"},{"name":"moveProjectToFolder","comment":"移动工程到文件夹","visibility":"public"},{"name":"copyProject","comment":"复制工程"},{"name":"getAllProjectsUuid","comment":"获取所有工程的 UUID","visibility":"public"},{"name":"getProjectInfo","comment":"获取工程属性","visibility":"public"},{"name":"getCurrentProjectInfo","comment":"获取当前工程的详细属性","visibility":"public"},{"name":"deleteProject","comment":"删除工程"},{"name":"waitForReady","comment":"等待工程就绪","visibility":"beta"},{"name":"getReadyState","comment":"获取工程的就绪状态","visibility":"beta"}],"callPath":"dmt_Project"},"logs":[],"durationMs":452}


PS C:\Users\User> easyeda-pro session close --session $S --destroy | Out-Host

{"ok":true,"value":{"sessionId":"374efcd5-3462-4103-9f72-3385fcd916c7","status":"closed","headless":false},"logs":[],"durationMs":30}
```



---



## M1: Windows 単体での CLI 自動操作 PoC

- [x] Windows ターミナルで `easyeda-pro open` を実行し、`ok: true` と `sessionId` が返ることを確認した
- [x] 取得した `sessionId` を使い、以下のコマンドで内部 API が実行できることを確認した
  ```bat
  easyeda-pro invoke --session <SESSION_ID> --ext-uuid eda --code "return eda.sys_Environment.getEditorCurrentVersion()"
  ```
- [x] `easyeda-pro doc api` で API クラス一覧が JSON で返ることを確認した
- [x] `easyeda-pro session list` で起動中セッションが表示されることを確認した
- [x] `easyeda-pro session close --session <SESSION_ID> --destroy` で CLI が開いたウィンドウが正常に閉じることを確認した

---

## M2: 非・常駐 `.bat` ランチャー作成と Ubuntu ⇄ Windows 疎通確認

### 接続前提の準備（基本案：SSH リバーストンネルの場合）
- [x] Windows 側で `ssh -V` が実行できることを確認した（Windows 標準 OpenSSH クライアント）
- [x] Ubuntu 側で SSH 接続を受け付けられる状態（`sshd` 稼働・LAN 内到達可能）であることを確認した（`AG-G3` / port 2222 で LXD コンテナ直着地）
- [x] Windows → Ubuntu へ **SSH 鍵認証**（パスワード入力なし、または鍵エージェント経由）でログインできることを確認した
- [ ] *(代替案を選ぶ場合・今回不要)* Node.js（v20 以上推奨）を Windows に導入し、`node -v` を確認した

### 非・常駐ランチャー作成と検証
- [x] MCP 認証用のランダムトークンを生成し、Git 管理外のローカル設定（Windows `bridge.config.local.ps1` / Ubuntu `~/.config/easyeda-bridge/token`）に保管した
- [x] 作業時だけ起動する `.bat`（`win/start_easyeda_bridge.bat` ＋ `start_easyeda_bridge.ps1`）を作成した
  - [x] EasyEDA Pro GUI が起動していることの確認（`doctor`）を含む
  - [x] `easyeda-pro --mcp http --port 3030 --token <TOKEN>` を起動できる
  - [x] `ssh -N -R 127.0.0.1:3030:127.0.0.1:3030 AG-G3` をフォアグラウンドで接続できる
- [x] `.bat` 起動状態で Ubuntu 側から `127.0.0.1:3030` への疎通を確認した（[result/20261126_1502.md](result/20261126_1502.md)）
- [x] トークンなしのリクエストが `401` で拒否され、正しいトークンで `initialize` / `tools/list` が `200` 応答することを確認した
- [x] **非・常駐確認（最重要）**:
  - [x] `.bat` を `Ctrl+C` で終了した（[result/20261126_1457.md](result/20261126_1457.md)）
  - [x] ランチャーが起動した `--mcp` プロセス木のみ終了し、ポート `127.0.0.1:3030` が閉鎖されることを確認した
  - [x] Windows 側に不要な常駐サービス（`sshd` 等）を増やしていないことを確認した
  - [x] 終了後、Ubuntu 側から `127.0.0.1:3030` へ接続できなくなることを確認した

---

## M3: Ubuntu エージェント側への MCP 統合と運用ルール整備

- [x] Ubuntu 側の `~/.gemini/config/mcp_config.json` へ EasyEDA Pro MCP の接続設定を記述した
- [x] オンデマンド非・常駐運用に対応したゼロ依存 Node.js stdio ブリッジ（[ubuntu/easyeda_mcp_stdio.mjs](ubuntu/easyeda_mcp_stdio.mjs)）を作成し、ライブ／オフライン待機の両モードを検証した
- [ ] Antigravity UI の **Additional Options (...) > MCP Servers** で `easyeda-pro` が有効になっていることをユーザー側で目視確認する
- [x] stdio ブリッジ経由で以下の読み取り系操作がエラーなく実行できた：
  - [x] `doctor`（接続状態の確認：`connected: true`）
  - [x] `session.list`（セッション一覧の確認）
  - [x] `doc.api`（`SCH_PrimitiveComponent` の API リファレンス参照・文字化けなし）
- [x] 公式 `cli-for-ai.md` の安全規約を [agent_rules.md](agent_rules.md) にまとめた

---

## M4: 「Hello Circuit」自動構築 PoC

- [x] 本番データと分離したテスト用プロジェクト（`Agent_Hello_Circuit` / `.eprj3`）を自動作成した
- [x] Ubuntu のエージェントからの操作だけで以下の自動構築ループを完走した：
  - [x] LCSC ライブラリからアクティブな JLCPCB Basic Part（`C21190` / `C25804` / `C14663` / `C2286`）を検索できた
  - [x] 回路図シートに部品 5 点（`R1`, `R2`, `C1`, `R3`, `LED1`）を配置し、Designator を設定できた
  - [x] `getAllPinsByPrimitiveId()` で各ピンの絶対座標・極性（`LED1` Pin1=`A` 上 / Pin2=`K` 下）を取得できた
  - [x] ピン座標に基づいて `SCH_PrimitiveWire` で直交配線（11 セグメント、T 字接合分割）できた
  - [x] `VCC` / `GND` ネットフラグを配置できた
  - [x] 保存・反映待ち（8 秒）の後に `SCH_Drc.check(true, false, true)` / `check(true, false, false)` を実行し、**厳格モード合格・違反 0 件**を確認した
  - [x] ネットリスト（[result/hello_circuit_netlist.enet](result/hello_circuit_netlist.enet)）・BOM（[result/hello_circuit_bom.csv](result/hello_circuit_bom.csv)）・回路図 PNG（[result/hello_circuit_sch.png](result/hello_circuit_sch.png)）を書き出し、目視＆ネット検証まで完了した
- [x] Windows 側の EasyEDA Pro 画面で、生成された回路図の見た目・接続を人間が目視確認した
- [x] *(追加 PoC・案 A)* 同じテスト回路を PCB 同期→外形 `1000×800 mil`→配置→15 mil 配線→GND ベタ→PCB DRC 違反 0 件・未配線 0 本→PNG / Gerber / CPL 書き出しまで完走した
- [ ] 作業完了後、不要な CLI セッションを `session close --destroy` で閉じ、`.bat` も正常終了した

---

## M5: ADX 設計ワークフローへの応用

- [ ] ADX 固有仕様（[ADX_PINOUT.md](../../docs/spec/ADX_PINOUT.md) / 8858 フォームファクタ）をエージェントが参照する導線を整備した
- [ ] 既存回路（テストコピー）に対する読み取り専用レビュー（ネット確認・BOM 集計・ERC/DRC チェック）を試した
- [ ] 小規模な ADX CARD 回路図のドラフト自動生成を検証した
- [ ] 繰り返し使う定型処理（20 ピン IDC コネクタ配置、保護回路ブロック等）のテンプレート化または `.eext` 拡張化の要否を判断した

---

## 日常運用チートシート（毎回セッション時）

### 開始時
- [ ] Windows で EasyEDA Pro デスクトップ版を起動する
- [ ] Windows で `start_easyeda_bridge.bat` をダブルクリック起動する
- [ ] Ubuntu のエージェントから `doctor` で `connected: true` を確認する

### 終了時
- [ ] エージェントが作成したセッションを保存・クローズ（`session close --destroy`）する
- [ ] Windows の `.bat` ウィンドウを `Ctrl+C`（または閉じる）で停止する
- [ ] 必要に応じてテスト結果・出力物を Git に整理する（トークン等の秘密情報が含まれないことを確認）
