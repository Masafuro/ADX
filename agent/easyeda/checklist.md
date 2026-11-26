# EasyEDA Agent 環境構築チェックリスト

[milestone_plan.md](milestone_plan.md) に沿って進めるための実作業チェックリストです。  
各マイルストーンの項目を上から順に確認し、問題があれば切り分けて対処します。

> **注意**: 接続用トークンや秘密鍵パスフレーズなどの機密情報は、このファイルや Git 管理下には記載しないでください。

---

## 環境メモ（作業しながら記入）

- **EasyEDA Pro バージョン**: `（例: 4.1.70 / International）`
- **EasyEDA Pro 実行ファイルパス**: `（例: C:\Program Files\EasyEDA-Pro\easyeda-pro.exe）`
- **ローカルプロジェクト保存先**: `（例: C:\Users\<user>\Documents\EasyEDA-Pro\projects）`
- **Node.js バージョン（Windows / Ubuntu）**: `Windows: v__ / Ubuntu: v__`
- **採用接続方式**: `[ ] SSH リバーストンネル（推奨）` / `[ ] Node.js フォアグラウンド中継`
- **使用ローカルポート**: `3030`（変更時はここを更新）

---

## M0: Windows 側への EasyEDA Pro デスクトップ版導入

- [x] [EasyEDA ダウンロードページ](https://easyeda.com/page/download) から **EasyEDA Pro デスクトップ版（International 版 / v4.1.70 以上）** を取得・インストールした
- [x] 既存の Web 版アカウントで EasyEDA Pro にログインできた
- [x] 半オフライン／ローカル保存モードの設定と、`Documents\EasyEDA-Pro\projects` フォルダの存在を確認した
- [x] コマンドプロンプト / PowerShell で `easyeda-pro --help` が実行でき、バージョン情報が表示された
  - *(パスが通っていない場合はインストール先フォルダを環境変数 `PATH` に追加)*
- [ ] `easyeda-pro --mcp --help` を実行し、`--mcp http` で利用可能なオプション（`--port` / `--token` / ホスト指定可否など）を確認・記録した
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

- [ ] Windows ターミナルで `easyeda-pro open` を実行し、`ok: true` と `sessionId` が返ることを確認した
- [ ] 取得した `sessionId` を使い、以下のコマンドで内部 API が実行できることを確認した
  ```bat
  easyeda-pro invoke --session <SESSION_ID> --ext-uuid eda --code "return eda.sys_Environment.getEditorCurrentVersion()"
  ```
- [ ] `easyeda-pro doc api` で API クラス一覧が JSON で返ることを確認した
- [ ] `easyeda-pro session list` で起動中セッションが表示されることを確認した
- [ ] `easyeda-pro session close --session <SESSION_ID> --destroy` で CLI が開いたウィンドウが正常に閉じることを確認した

---

## M2: 非・常駐 `.bat` ランチャー作成と Ubuntu ⇄ Windows 疎通確認

### 接続前提の準備（基本案：SSH リバーストンネルの場合）
- [ ] Windows 側で `ssh -V` が実行できることを確認した（Windows 標準 OpenSSH クライアント）
- [ ] Ubuntu 側で SSH 接続を受け付けられる状態（`sshd` 稼働・LAN 内到達可能）であることを確認した
- [ ] Windows → Ubuntu へ **SSH 鍵認証**（パスワード入力なし、または鍵エージェント経由）でログインできることを確認した
- [ ] *(代替案を選ぶ場合)* Node.js（v20 以上推奨）を Windows に導入し、`node -v` を確認した

### 非・常駐ランチャー作成と検証
- [ ] MCP 認証用のランダムトークンを生成し、Git 管理外のローカル設定（例：Windows ローカルの環境変数設定ファイル等）に保管した
- [ ] 作業時だけ起動する `.bat`（例：`start_easyeda_bridge.bat`）を作成した
  - [ ] EasyEDA Pro GUI が起動していることの確認（または起動誘導）を含む
  - [ ] `easyeda-pro --mcp http --port 3030 --token <TOKEN>` を起動できる
  - [ ] `ssh -N -R 127.0.0.1:3030:127.0.0.1:3030 <user>@<ubuntu-ip>`（または代替中継）をフォアグラウンドで接続できる
- [ ] `.bat` 起動状態で Ubuntu 側から `127.0.0.1:3030` への疎通を確認した
- [ ] トークンなし／誤トークンのリクエストが拒否され、正しいトークンで応答することを確認した
- [ ] **非・常駐確認（最重要）**:
  - [ ] `.bat` を `Ctrl+C` またはウィンドウを閉じて終了した
  - [ ] タスクマネージャ（または `tasklist`）で `ssh.exe` および `--mcp` 用プロセスが残っていないことを確認した
  - [ ] Windows のサービス一覧で不要な常駐サービス（`sshd` 等）が増えていないことを確認した
  - [ ] 終了後、Ubuntu 側から `127.0.0.1:3030` へ接続できなくなることを確認した

---

## M3: Ubuntu エージェント側への MCP 統合と運用ルール整備

- [ ] Ubuntu 側の `~/.gemini/config/mcp_config.json`（または専用プラグイン設定）へ EasyEDA Pro MCP の接続設定を記述した
- [ ] Streamable HTTP で直接接続できるか検証した
  - [ ] *(直接接続に制約がある場合)* Ubuntu 側に薄い Node.js stdio ブリッジを用意し、MCP ツールとして認識させた
- [ ] エージェントの MCP サーバー一覧で EasyEDA Pro ツールが表示されることを確認した
- [ ] エージェントから以下の読み取り系操作がエラーなく実行できた：
  - [ ] `doctor`（接続状態の確認）
  - [ ] `session.list`（セッション一覧の確認）
  - [ ] `doc api`（`SCH_PrimitiveComponent` 等の API リファレンス参照）
- [ ] 公式 `cli-for-ai.md` の安全規約（`--ext-uuid eda`、ピン座標実測、ウェイト、セッションクローズ、テストプロジェクト限定ルール）を `agent/easyeda/` 配下にまとめた

---

## M4: 「Hello Circuit」自動構築 PoC

- [ ] 本番データと分離したテスト用プロジェクト（例：`Agent_Hello_Circuit.eprj3`）を用意した
- [ ] Ubuntu のエージェントへの指示だけで以下の自動構築ループを完走した：
  - [ ] LCSC ライブラリからテスト用部品（抵抗・コンデンサ・LED 等）を検索できた
  - [ ] 回路図シートに部品を配置し、Designator（`R1`, `C1` 等）を設定できた
  - [ ] `getAllPinsByPrimitiveId()` で各ピンの絶対座標を取得できた
  - [ ] ピン座標に基づいて `SCH_PrimitiveWire` で配線できた
  - [ ] `VCC` / `GND` ネットフラグを配置できた
  - [ ] 保存・反映待ち（5〜10 秒）の後に `SCH_Drc.check(true, false, true)` を実行し、エラー 0 件を確認した
  - [ ] ネットリスト・BOM・回路図 PNG スクリーンショットを書き出せた
- [ ] Windows 側の EasyEDA Pro 画面で、生成された回路図の見た目・接続を人間が目視確認した
- [ ] 不要な CLI セッションを `session close --destroy` で閉じ、`.bat` も正常終了できた

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
