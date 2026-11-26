# EasyEDA Agent 環境構築マイルストーン計画

本ドキュメントは、[concept.md](concept.md) の構想に基づき、Ubuntu 上の AI エージェントから Windows 開発 PC 上の EasyEDA Pro を操作して回路構築を行うための環境構築マイルストーン計画です。  
実作業の進捗管理と確認項目は [checklist.md](checklist.md) を参照してください。

---

## 1. 方針・前提条件

- **採用構成（構成 1）**:
  - **Windows 開発 PC**: EasyEDA Pro デスクトップ版を実行し、人間がリアルタイムに回路図・PCB を視認・レビューできる環境とする。
  - **Ubuntu PC**: AI エージェント（Antigravity 等）を稼働させ、LAN 経由で EasyEDA Pro に接続して設計操作を行う。
- **非・常駐ポリシー（重要）**:
  - Windows 側に常駐サービス（`sshd` サービス常駐やバックグラウンド常駐デーモン等）は置かない。
  - 作業時のみ `.bat` スクリプトをフォアグラウンド起動し、作業終了時にウィンドウを閉じる（または `Ctrl+C`）ことで接続・プロセスを完全に停止できる設計とする。
- **標準機能優先**:
  - EasyEDA Pro デスクトップ版（v4.1.60 以降・推奨 **v4.1.70 以上**）に内蔵されている公式 CLI（`easyeda-pro`）および公式 MCP サーバー機能（`--mcp http` / `--mcp stdio`）を優先採用する。
  - 初期構築段階ではサードパーティ製 `.eext` 拡張に依存せず、必要に応じて Node.js や公式 SDK（`easyeda-api-sdk`）を段階導入する。
- **既存資産（Web 版）の保護**:
  - これまで Web 版で作成した本番プロジェクトには初期検証で触れず、M0〜M4 はテスト用の新規ローカルプロジェクト（`.eprj3` 形式）で検証する。

---

## 2. 想定アーキテクチャ（非・常駐 `.bat` 起動型）

### 基本案：SSH リバーストンネル＋公式 HTTP MCP（推奨）

```text
[Ubuntu PC (エージェント)]                       [Windows 開発 PC]
  Antigravity / MCP クライアント                   start_easyeda_bridge.bat（作業時のみ起動）
       │                                            ├─ 1. easyeda-pro --mcp http --port 3030 --token <TOKEN>
       │  http://127.0.0.1:3030                     │       └─ EasyEDA Pro GUI（ローカルブリッジ接続）
       └─── SSH リバーストンネル (-R) ◀─────────────┴─ 2. ssh -N -R 127.0.0.1:3030:127.0.0.1:3030 <user>@<ubuntu-ip>
```

- **Windows 側**: OS 標準の OpenSSH **クライアント**（`ssh.exe`）のみを使用。常駐サービス不要、Windows ファイアウォールの受信ポート開放も不要。
- **セキュリティ**: EasyEDA の操作ポートは Windows / Ubuntu ともに `127.0.0.1` に閉じ、LAN 間通信は SSH 暗号化＋トークン認証で二重保護する。
- **終了性**: `.bat` を終了すればトンネルと MCP サーバーが同時に停止し、裏に常駐プロセスを残さない。

### 代替案（Ubuntu 側で SSH 受信を使わない場合）

- Windows の `.bat` から `easyeda-pro --mcp http` を起動し、必要であれば同一 `.bat` 内で Node.js のフォアグラウンド HTTP リバースプロキシを併用して LAN 内の Ubuntu PC からのみアクセス可能にする。
- この場合も `.bat` 終了時にプロセスをすべて停止させ、常駐化しない。

---

## 3. マイルストーン一覧

| ID | マイルストーン名 | 主な目的 | 完了条件（Exit Criteria） |
| :--- | :--- | :--- | :--- |
| **M0** | Windows 側 EasyEDA Pro デスクトップ版導入 | ローカル実行基盤と公式 CLI の疎通確保 | `easyeda-pro doctor` が `ok: true` / `connected: true` を返す |
| **M1** | Windows 単体での CLI 自動操作 PoC | LAN を跨ぐ前に EasyEDA 内部 API 実行を検証 | `open` → `invoke` → `session close --destroy` が成功 |
| **M2** | 非・常駐 `.bat` ランチャー作成と LAN 疎通 | Windows ⇄ Ubuntu 間の安全なオンデマンド接続確立 | `.bat` 起動中のみ Ubuntu から応答し、終了後は完全遮断 |
| **M3** | Ubuntu エージェントへの MCP 統合・ルール整備 | エージェントからツールとして EasyEDA を操作可能にする | エージェント経由で `doctor` / `session.list` / `doc api` 実行成功 |
| **M4** | 「Hello Circuit」自動構築 PoC | 最小回路の生成〜検証〜出力までの一気通貫実証 | Windows 画面に回路描画され、DRC エラー 0 で成果物出力 |
| **M5** | ADX 設計ワークフローへの応用 | 実プロジェクト（CARD / CORE）設計・レビューへの展開 | ADX 仕様を参照した回路ドラフト生成または自動チェック実施 |

---

## 4. 各マイルストーン詳細

### M0: Windows 側への EasyEDA Pro デスクトップ版導入
- **狙い**: Web 版からデスクトップ版（公式 CLI / ローカルブリッジ対応版）へ作業基盤を拡張する。
- **実施内容**:
  1. EasyEDA Pro デスクトップ版（International 版 / v4.1.70 以上推奨）を Windows PC にインストール。
  2. 既存の EasyEDA アカウントでサインインし、Web 版プロジェクトの参照可否とローカル保存先（`%USERPROFILE%\Documents\EasyEDA-Pro\projects` 等、半オフラインモード）を確認。
  3. コマンドプロンプト / PowerShell から `easyeda-pro --help`・`easyeda-pro --mcp --help`・`easyeda-pro doctor` を実行し、CLI のパスとオプション仕様を確認。
- **成果物**: 動作する Windows 版 EasyEDA Pro 環境、CLI バージョンおよび `--mcp` オプションのメモ。

### M1: Windows 単体での CLI 自動操作 PoC
- **狙い**: ネットワーク層を絡める前に、EasyEDA Pro 本体と CLI の挙動を切り分けて検証する。
- **実施内容**:
  1. `easyeda-pro open` でエディタセッションを起動し、`sessionId` を取得。
  2. `easyeda-pro invoke --session <ID> --ext-uuid eda --code "return eda.sys_Environment.getEditorCurrentVersion()"` を実行し、`eda.*` API 実行が通ることを確認。
  3. `easyeda-pro doc api` で API リファレンス検索を確認。
  4. `easyeda-pro session close --session <ID> --destroy` でウィンドウをクリーンに閉じる。
- **成果物**: Windows ローカルでの最小実行ログ（JSON レスポンス）。

### M2: 非・常駐 `.bat` ランチャー作成と Ubuntu ⇄ Windows 疎通確認
- **狙い**: 「作業時だけ起動・終了時ゼロ常駐」の接続フローを確立する。
- **実施内容**:
  1. 接続方式を確定（推奨：Windows → Ubuntu の SSH 鍵認証＋リバーストンネル `ssh -N -R`、代替：Node.js フォアグラウンド中継）。
  2. トークンを環境変数または `.gitignore` 対象のローカル設定ファイルから読み込み、`easyeda-pro --mcp http --port 3030 --token <TOKEN>` とトンネルをまとめて起動・終了する `.bat`（または `.ps1` 呼び出し `.bat`）を作成。
  3. `.bat` 起動状態で Ubuntu 側から `curl` によりエンドポイント疎通およびトークン認証を確認。
  4. `.bat` を閉じた後、Windows 側に MCP / SSH の残存プロセスがないこと、および Ubuntu 側から接続不可になることを確認。
- **成果物**: Windows 側起動用 `.bat` スクリプト（トークンはリポジトリにコミットしない）、疎通・停止確認結果。

### M3: Ubuntu エージェント側への MCP 統合と運用ルール整備
- **狙い**: Ubuntu 上のエージェント（Antigravity 等）から EasyEDA Pro を安全かつ安定して操作できるようにする。
- **実施内容**:
  1. Ubuntu 側の `~/.gemini/config/mcp_config.json`（または専用プラグイン設定）に EasyEDA Pro MCP サーバーを登録。
  2. Antigravity の **Streamable HTTP** トランスポートと EasyEDA Pro の `--mcp http` 間の互換性（エンドポイント URL、認証ヘッダ形式、プロトコル版）を検証。差異がある場合は Ubuntu 側に薄い Node.js stdio ブリッジを挟んで吸収する。
  3. 公式 `cli-for-ai.md` の重要制約（`--ext-uuid eda` 付与、ピン座標は `getAllPinsByPrimitiveId()` で取得、編集後 5〜10 秒ウェイト、`session close --destroy`、本番プロジェクト外への書き込み禁止等）を `agent/easyeda/` 配下のルール／スキルとしてドキュメント化。
- **成果物**: MCP 接続設定、エージェント用操作ガイド／スキル。

### M4: 「Hello Circuit」自動構築 PoC
- **狙い**: エージェントからの自然言語指示で、実際に回路図が構築・検証できることを実証する。
- **実施内容**:
  1. テスト用の空プロジェクト（`.eprj3`）に対して、最小回路（例：抵抗分圧＋RC フィルタ、または LED＋電流制限抵抗＋VCC/GND）を構築させる。
  2. 以下の全工程をエージェント経由で完走させる：
     - LCSC ライブラリからの部品検索
     - シンボル配置とリファレンス（Designator）設定
     - `getAllPinsByPrimitiveId()` による絶対ピン座標取得
     - 配線（`SCH_PrimitiveWire`）と電源/GND ネットフラグ配置
     - 保存・ウェイト後の `SCH_Drc.check()` 実行（エラー 0 を確認）
     - ネットリスト・BOM・スクリーンショット PNG の書き出し
  3. Windows 側の EasyEDA Pro 画面上で回路図が意図通り描画されていることを人間が目視確認する。
- **成果物**: 自動生成されたテスト回路プロジェクト、DRC 結果、スクリーンショット PNG。

### M5: ADX 設計ワークフローへの応用（発展フェーズ）
- **狙い**: 実験環境を ADX ハードウェア開発の実務に結びつける。
- **実施内容**:
  1. [ADX 20 ピンバス仕様](../../docs/spec/ADX_PINOUT.md) や 8858 フォームファクタ仕様をエージェントの前提知識として参照させる。
  2. まずはリスクの低いタスク（既存回路のネット・BOM・DRC レビュー、小規模なテスト用 CARD 回路図ドラフト生成）から実戦投入する。
  3. 反復作業が定型化したら、Node.js + `easyeda-api-sdk` による ADX 専用カスタム拡張（`.eext`）や専用テンプレートの導入を検討する。
- **成果物**: ADX 向けエージェント設計ワークフロー定義。

---

## 5. 想定リスクと対策

| リスク | 想定される症状 | 対策 |
| :--- | :--- | :--- |
| `--mcp http` のバインド制約 | `127.0.0.1` にしかバインドできず LAN から直接繋がらない | 基本案の SSH リバーストンネル（`-R`）なら `127.0.0.1` のままで問題なく接続可能 |
| MCP トランスポート / 認証ヘッダ差異 | Antigravity の Streamable HTTP 設定でヘッダ指定や旧 SSE 方式が噛み合わない | Ubuntu 側に薄い Node.js stdio ラッパー（または `mcp-remote`）を置いて吸収する |
| `.bat` 終了後の子プロセス残存 | コンソールを閉じても `easyeda-pro --mcp` や `ssh` がバックグラウンドに残る | `.bat` 終了トラップ / 子プロセス制御を設計し、M2 でタスクマネージャ確認を必須化 |
| 座標計算ミスによる配線未接続 | ピンに届かない・微妙にずれて DRC エラーになる | ピン座標は手計算せず、必ず `getAllPinsByPrimitiveId()` の実測値から配線する |
| トークン・認証情報の誤コミット | `.bat` や `mcp_config.json` に書いたトークンを Git に push してしまう | トークンは `.env.local` 等（`.gitignore` 対象）または `~/.gemini/config/` に分離する |

---

## 6. 参考資料

- EasyEDA Pro ダウンロード: https://easyeda.com/page/download
- EasyEDA Pro Command-Line Client（公式ガイド）: https://prodocs.easyeda.com/en/api/guide/cli.html
- EasyEDA Pro CLI for AI（公式 AI エージェント向け仕様）: https://prodocs.easyeda.com/storage/texts/api/guide/cli-for-ai.md
