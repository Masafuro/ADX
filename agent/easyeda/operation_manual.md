# EasyEDA Pro Agent 実行マニュアル（日常運用ガイド）

初期導入（[setup_manual.md](setup_manual.md)）が完了した後の、普段の起動・回路設計実行・安全な終了手順をまとめた運用マニュアルです。

---

## 1. セッション開始手順（所要 約 30 秒）

1. **Windows 側: EasyEDA Pro デスクトップ版を起動**
   - 半オフラインモード（Half-Offline Mode）になっていることを確認します。
2. **Windows 側: 非・常駐ブリッジを起動**
   - `%USERPROFILE%\easyeda-bridge\start_easyeda_bridge.bat` をダブルクリック（または PowerShell から `.\start_easyeda_bridge.bat`）で起動します。
   - コンソールに以下が順番に表示されることを確認します：
     - `[1/5]` Config loaded（`target=AG-G3`, `port=3030`）
     - `[2/5]` EasyEDA Pro GUI connected (`bridgeVersion=...`)
     - `[3/5]` MCP server is listening on `127.0.0.1:3030`
     - `[4/5]` Windows local MCP self-test passed on path `/mcp`
     - `[5/5]` `Authenticated to ... using "publickey".`
   - このウィンドウは作業中そのまま最小化等で開いたままにします。
3. **Ubuntu 側: 接続確認（任意・推奨）**
   - ターミナルから確認する場合：
     ```bash
     /home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/ubuntu/check_bridge.sh 3030
     ```
     （`SUMMARY` が 4 項目すべて `PASS` なら準備完了）
   - Antigravity のチャットから確認する場合：
     > 「EasyEDA Pro の `doctor` と `session.list` を確認して、接続状態を教えてください。」

---

## 2. エージェントから EasyEDA Pro を操作する

### 2.1 チャットからの基本的な依頼テンプレート

操作ミスや本番プロジェクト破損を防ぐため、以下の 4 点を含めて指示するのがおすすめです（詳細は [agent_rules.md](agent_rules.md)）。

1. **対象プロジェクト**（新規作成か既存変更か／プロジェクト名）
2. **作りたい回路・仕様**（部品条件：JLCPCB Basic Part 優先など）
3. **検証条件**（SCH DRC / PCB DRC 違反 0 件、ネットリスト確認、未配線ラッツネスト 0 本）
4. **確認用成果物**（PNG スクリーンショット、BOM、Gerber 等）

#### プロンプト例（新規テスト回路の構築）
> `agent/easyeda/agent_rules.md` に従って EasyEDA Pro に接続し、ローカルにテスト用プロジェクト `Test_LED_Card`（`.eprj3`）を新規作成してください。  
> JLCPCB Basic Part を使って回路図を作成し、ピン座標実測で配線、保存後ウェイトを置いて SCH DRC 違反 0 件を確認し、回路図 PNG と BOM を `agent/easyeda/result/` へ出力してください。

#### プロンプト例（回路図 → PCB 展開）
> 現在のテストプロジェクトの回路図を PCB に同期（`importChanges`）し、外形 `25.4 × 20.32 mm` で部品配置・配線・GND ベタ塗りを行ってください。PCB DRC 違反 0 件・未配線 0 本を確認したら、PCB プレビュー画像と Gerber ZIP を出力してください。

---

### 2.2 ターミナルから手動コマンド・スクリプトを実行する（`mcp_call.mjs`）

再現性のある定型処理やデバッグには、Ubuntu 側の [mcp_call.mjs](ubuntu/mcp_call.mjs) が使えます。

```bash
HELPER="/home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/ubuntu/mcp_call.mjs"

# 1. 接続・セッション確認
node "$HELPER" doctor '{}'
node "$HELPER" session.list '{}'

# 2. 新しいエディタセッションを開く（返ってきた sessionId を控える）
node "$HELPER" open '{}'

# 3. API リファレンスを調べる（文字化けなしの UTF-8 JSON）
node "$HELPER" doc.api '{"className":"SCH_PrimitiveComponent","methodName":"create"}'

# 4. JS スクリプトを実行する（--ext-uuid eda 自動付与）
node "$HELPER" invoke --session "<SESSION_ID>" --file /path/to/script.js --timeout 60000

# 5. CLI で開いたセッションを閉じる
node "$HELPER" session.close '{"session":"<SESSION_ID>","destroy":true}'
```

#### サンプル PoC の再現実行（参考）
[poc_hello_circuit/](poc_hello_circuit/) 配下に検証済みスクリプト一式があります：
1. `step1_create_and_place.js`: プロジェクト作成・LCSC 部品配置・ピン座標実測
2. `step2_wire_drc_export.js`: 直交配線・ネットフラグ・SCH DRC・PNG / BOM / ネットリスト書き出し
3. `step3_pcb_import_place.js`: PCB 同期・外形枠作成・部品配置・パッド座標実測
4. `step4_pcb_route_drc_export.js`: トップ層配線・GND ベタ塗り・PCB DRC・PNG / Gerber / CPL 書き出し

---

## 3. セッション終了手順（非・常駐を守るクリーンアップ）

作業が終わったら、以下の順番で確実に後片付けします。

1. **エージェントが開いた CLI セッションを閉じる**
   - 画面確認が終わったら、エージェントへ「CLI で開いた EasyEDA セッションを保存して閉じてください」と指示するか、手動で実行します：
     ```bash
     node /home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/ubuntu/mcp_call.mjs \
       session.close '{"session":"<SESSION_ID>","destroy":true}'
     ```
   - *(※ ユーザーが手動起動した EasyEDA Pro GUI 本体はそのまま残ります。)*
2. **Windows 側の `.bat` ブリッジを `Ctrl+C` で停止する**
   - **重要**: ウィンドウ右上の **`×` ボタンではなく、`Ctrl+C`** を押して終了してください（`finally` クリーンアップが確実に走ります）。
   - コンソールに以下が出たことを確認してからキーを押して閉じます：
     ```text
     [CLEANUP] Stopping MCP server started by this launcher...
               Stopping PID tree rooted at PID=...
               [OK]   Port 127.0.0.1:3030 is closed (non-resident verified).
     ```
3. **閉鎖確認（必要に応じて）**
   - Ubuntu 側で `check_bridge.sh 3030` を実行し、`[1/4] Tunnel port : FAIL` になっていれば完全停止です。

---

## 4. メンテナンス・トラブル即応表

### Windows 側ランチャーの更新方法
リポジトリ側の `start_easyeda_bridge.bat` / `.ps1` を更新した際は、`bridge.config.local.ps1` を壊さずスクリプト本体だけ上書きできます：

```powershell
scp AG-G3:/home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/win/start_easyeda_bridge.bat `
    AG-G3:/home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/win/start_easyeda_bridge.ps1 `
    "$env:USERPROFILE\easyeda-bridge\"
```

### トークンのローテーション（変更したいとき）
1. Windows の `bridge.config.local.ps1` の `$MCP_TOKEN` を新しいランダム文字列に変更。
2. Ubuntu の `~/.config/easyeda-bridge/token` を同じ文字列で上書き（`chmod 600` 維持）。
3. Antigravity の再起動は不要です（`easyeda_mcp_stdio.mjs` が都度トークンファイルを読み込みます）。

### よくある運用エラーと対処

| メッセージ / 症状 | 原因 | 対処 |
| :--- | :--- | :--- |
| `Cannot reach EasyEDA Pro bridge at http://127.0.0.1:3030/mcp (ECONNREFUSED)` | Windows の `start_easyeda_bridge.bat` が起動していない | Windows で `start_easyeda_bridge.bat` を起動して再実行（Antigravity 再起動不要） |
| `EasyEDA MCP HTTP 401: Unauthorized` | Windows と Ubuntu のトークンが不一致 | `bridge.config.local.ps1` と `~/.config/easyeda-bridge/token` の内容・末尾改行を確認 |
| `Port 127.0.0.1:3030 is already in use`（Windows 起動時） | 前回 `×` ボタンで閉じた等で古い MCP プロセスが残っている | タスクマネージャで該当の `--mcp` プロセスのみ終了（GUI 本体は残す）するか、PC 再起動 |
| 編集直後の DRC で誤検知・古い結果が返る | エディタ内部の非同期更新が未完了 | `save()` 後に 5〜10 秒ウェイトを置いてから `check()` を再実行 |

---

## 関連ドキュメント
- 初期導入手順: [setup_manual.md](setup_manual.md)
- 安全・再現性ルール: [agent_rules.md](agent_rules.md)
- 現状サマリー・PoC 結果: [status_summary.md](status_summary.md)
