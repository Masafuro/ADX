# EasyEDA Pro Agent 導入マニュアル

Ubuntu（LXD コンテナ）上の AI エージェント（Antigravity 等）から、ローカルネットワーク上の Windows 開発 PC で動作する **EasyEDA Pro デスクトップ版** を操作するための初期構築手順書です。

- **構成方針**: Windows 側に常駐サービス（`sshd` 等）を置かず、作業時のみ `.bat` を起動して **SSH リバーストンネル** で安全に接続する「非・常駐型」構成を採用します。
- **対象読者**: 本環境を別 PC や新環境へ再現構築する開発者。日常の起動・終了・操作方法は [operation_manual.md](operation_manual.md) を参照してください。

---

## 1. 前提条件・動作検証済み環境

| 項目 | Windows 開発 PC | Ubuntu（LXD コンテナ） |
| :--- | :--- | :--- |
| OS | Windows 10 / 11 (x64) | Ubuntu 22.04+ (LXD コンテナ可) |
| 必須ソフト | EasyEDA Pro デスクトップ版（International 版 **v4.1.70 以上**、検証済み `4.1.105`） | Node.js **v20 以上**（検証済み `v20.20.2`）、`curl`、`python3` |
| SSH | OS 標準 OpenSSH **クライアント**（`ssh.exe` / サービス常駐不要） | `sshd` 稼働済み（Windows から鍵認証でログイン可能） |
| ファイアウォール | **受信ポート開放不要**（Windows からの発信 SSH のみ使用） | LAN 内から SSH 到達可能 |

---

## 2. Step 1: Windows 側への EasyEDA Pro デスクトップ版導入

### 2.1 インストールとログイン
1. [EasyEDA ダウンロードページ](https://easyeda.com/page/download) から **EasyEDA Pro デスクトップ版（International 版 / v4.1.70 以上）** をダウンロードし、インストールします。
2. 起動し、既存の EasyEDA アカウント（Web 版と共通）でログインします。

### 2.2 半オフラインモード（Half-Offline Mode）の確認・切替
エージェントのローカル `.eprj3` 操作や本番クラウドデータとの分離のため、**半オフラインモード**（プロジェクトはローカル保存、LCSC システムライブラリはオンライン参照）を使用します。

- スタートページ右上の **歯車アイコン（設定）**、または **スタートページ右クリック → Setting → Switch Run Mode**（あるいは上部メニュー **Setting → Desktop Client Setting → Running Mode Setting**）から **Half-Offline Mode（半オフラインモード）** に設定します。
- ローカルプロジェクトは既定で `%USERPROFILE%\Documents\EasyEDA-Pro\projects` に保存されます。

### 2.3 CLI 疎通確認（PowerShell）
EasyEDA Pro GUI を起動した状態で、Windows PowerShell を開いて確認します。

> **Tips**: `easyeda-pro.exe` は GUI サブシステム実行ファイルのため、PowerShell では末尾に **`| Out-Host`** を付けると完了まで待機して綺麗に出力されます。また日本語・中国語コメントの文字化けを防ぐ場合は `[Console]::OutputEncoding = [System.Text.Encoding]::UTF8` を先に実行します。

```powershell
# CLI バージョンとヘルプ確認（PATH が通っていない場合は C:\Program Files\easyeda-pro\ を PATH に追加）
easyeda-pro --help | Out-Host

# MCP サーバーオプション確認（--host / --port / --token があること）
easyeda-pro --mcp --help | Out-Host

# GUI ブリッジとの接続確認
easyeda-pro doctor | Out-Host
```

**期待結果**: `doctor` の JSON で `"ok": true` かつ `"connected": true`、`"versionMatch": true` が返ること。

---

## 3. Step 2: Windows → Ubuntu（LXD）SSH 鍵認証の確認

Windows PowerShell から、エージェントが動いている Ubuntu（または LXD コンテナ）へ **パスワード入力なし（鍵認証）** で直接ログインできることを確認します。

```powershell
# OpenSSH クライアントの存在確認
ssh -V

# 普段使っている SSH 接続先（~/.ssh/config の Host 名など、例: AG-G3）でワークスペースが見えるか確認
ssh AG-G3 "ls /home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda"
```

- **確認ポイント（LXD 環境の場合）**: コマンド結果に `checklist.md` や `win` フォルダが表示されれば、SSH が Ubuntu ホストではなく **LXD コンテナ内へ直接着地** しています。
- もし `~/.ssh/config` にカスタムポート（例: `Port 2222`）や鍵ファイル（`IdentityFile`）を設定している場合は、後続の `bridge.config.local.ps1` で `$UBUNTU_HOST` にその `Host` 名を指定します。

---

## 4. Step 3: Windows 非・常駐ランチャーの配置と設定

### 4.1 ランチャー一式のコピー
リポジトリの `agent/easyeda/win/` フォルダを Windows の任意のディレクトリ（例: `%USERPROFILE%\easyeda-bridge`）へコピーします。

```powershell
scp -r AG-G3:/home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/win "$env:USERPROFILE\easyeda-bridge"
cd "$env:USERPROFILE\easyeda-bridge"
```

### 4.2 認証トークンとローカル設定ファイルの作成
`bridge.config.example.ps1` を参考に、**`bridge.config.local.ps1`**（Git 管理外）を作成します。

> **重要（よくある落とし穴）**: `~/.ssh/config` の `Host` エイリアス（例: `AG-G3`）を使う場合、**`$UBUNTU_USER`・`$SSH_PORT`・`$SSH_KEY` は必ず空文字 `""` にしてください**。ここに `22` やホスト名を入れると `~/.ssh/config` のポート設定（例: 2222）や鍵指定を上書きして接続失敗します。

```powershell
$token = -join ((1..32) | ForEach-Object { '{0:x}' -f (Get-Random -Maximum 16) })
@"
`$UBUNTU_USER = ""
`$UBUNTU_HOST = "AG-G3"
`$SSH_PORT    = ""
`$SSH_KEY     = ""
`$MCP_PORT    = 3030
`$MCP_TOKEN   = "$token"
"@ | Set-Content -Encoding ASCII ".\bridge.config.local.ps1"

Write-Host "Generated Token (次の Step 4 で Ubuntu 側にも設定します): $token"
```

*(※ `~/.ssh/config` を使わず直接 IP 接続する場合は、`$UBUNTU_USER = "ubuntu"`・`$UBUNTU_HOST = "<Ubuntu LAN IP>"`・`$SSH_PORT = 22`・`$SSH_KEY = "$env:USERPROFILE\.ssh\id_ed25519"` のように明示指定してください。)*

---

## 5. Step 4: Ubuntu 側トークン保管と疎通テスト

### 5.1 トークンファイルの保存（Git 管理外・権限 600）
Ubuntu 側のターミナルで、Step 3 で生成した同じトークンを保存します。

```bash
mkdir -p ~/.config/easyeda-bridge && chmod 700 ~/.config/easyeda-bridge
printf '%s' '<Step 3 で生成したトークン>' > ~/.config/easyeda-bridge/token
chmod 600 ~/.config/easyeda-bridge/token
```

### 5.2 起動と 4 段階疎通確認
1. Windows で EasyEDA Pro GUI を起動した状態で、以下を実行します：
   ```powershell
   cd "$env:USERPROFILE\easyeda-bridge"
   .\start_easyeda_bridge.bat
   ```
   - `[1/5]`〜`[4/5]` が `[OK]` になり、`[5/5]` で `Authenticated to ... using "publickey".` と表示されれば接続完了です。
2. Ubuntu 側で診断スクリプトを実行します：
   ```bash
   /home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/ubuntu/check_bridge.sh 3030
   ```
   - 最後の `SUMMARY` が以下の通り全 4 項目 `PASS` になれば成功です：
     ```text
     SUMMARY
       [1/4] Tunnel port  : PASS
       [2/4] Auth reject  : PASS
       [3/4] Initialize   : PASS (/mcp)
       [4/4] tools/list   : PASS
     ```

### 5.3 非・常駐の停止確認
1. Windows の `.bat` ウィンドウで **`Ctrl+C`** を押します。
2. `[CLEANUP]` に `[OK] Port 127.0.0.1:3030 is closed (non-resident verified).` と表示されることを確認し、キーを押してウィンドウを閉じます。
3. Ubuntu 側でもう一度 `check_bridge.sh 3030` を実行し、`[1/4] Tunnel port : FAIL`（ポート閉鎖）になっていることを確認します。

---

## 6. Step 5: Ubuntu 側 Antigravity MCP サーバー登録

Ubuntu 側の `~/.gemini/config/mcp_config.json` に以下を記述します（トークンは `~/.config/easyeda-bridge/token` から動的に読むため、`mcp_config.json` には秘密情報を書きません）。

```json
{
  "mcpServers": {
    "easyeda-pro": {
      "command": "node",
      "args": [
        "/home/ubuntu/AgentWorkspace/Dev/github/ADX/agent/easyeda/ubuntu/easyeda_mcp_stdio.mjs"
      ]
    }
  }
}
```

### 登録後の動作確認
1. JSON 構文チェック：
   ```bash
   python3 -m json.tool ~/.gemini/config/mcp_config.json
   ```
2. Antigravity の UI から **Additional Options (...) > MCP Servers** を開き、`easyeda-pro` が登録されていることを確認します。
   - **ポイント**: [easyeda_mcp_stdio.mjs](ubuntu/easyeda_mcp_stdio.mjs) は **オフライン待機モード** を備えているため、Windows 側 `.bat` を閉じている状態で Antigravity を起動してもエラーにならず、`.bat` を起動した瞬間から再起動なしでツール呼び出しが可能になります。

---

## 7. トラブルシューティング（検証済み事例集）

| 症状 | 原因 | 対処 |
| :--- | :--- | :--- |
| `easyeda-pro doctor` が `connected: false` | EasyEDA Pro GUI が未起動、またはエディタセッション未開設 | EasyEDA Pro デスクトップ版を起動する（または `easyeda-pro open` を実行） |
| PowerShell でプロンプトが先に戻り JSON が後から出る | `easyeda-pro.exe` が GUI バイナリのため PowerShell が完了待機しない | コマンド末尾に `\| Out-Host` を付ける |
| PowerShell で `doc api` の説明文が文字化けする | PowerShell 5.1 既定の Shift-JIS（CP932）で UTF-8 を表示しているため | `[Console]::OutputEncoding = [System.Text.Encoding]::UTF8` を実行（なお MCP 経由では UTF-8 のため化けない） |
| `Warning: Identity file AG-G3 not accessible` | `bridge.config.local.ps1` でホスト名を誤って `$SSH_KEY` に書いている | `$UBUNTU_HOST = "AG-G3"` とし、`$SSH_KEY = ""` にする |
| `Permission denied (publickey)`（22 番へ接続） | `$SSH_PORT = 22` が `~/.ssh/config` のカスタムポート（例: 2222）を上書きしている | `bridge.config.local.ps1` で `$UBUNTU_USER = ""`・`$SSH_PORT = ""` にする |
| `错误：非全离线客户端环境无法调用本接口！` | 半オフライン環境で完全オフライン専用 API（`getLibrariesPaths()` 等）を呼んだ | 半オフライン時は `getLibrariesPaths()` を呼ばない（`getByLcscIds()` や `getCurrentProjectInfo()` を利用） |

---

## 関連ドキュメント
- 日常の起動・操作・終了手順: [operation_manual.md](operation_manual.md)
- エージェントの安全運用規約: [agent_rules.md](agent_rules.md)
- 現状サマリー・PoC 結果: [status_summary.md](status_summary.md)
