# EasyEDA Pro Agent 開発ドキュメント一覧

Ubuntu（LXD コンテナ）上の AI エージェントから、ローカルネットワーク内の Windows 開発 PC 上にある **EasyEDA Pro デスクトップ版** を安全（非・常駐）に操作し、回路図・PCB・製造データを自動構築するためのツール＆ドキュメント集です。

---

## ドキュメント案内

| ファイル | 役割・読むタイミング |
| :--- | :--- |
| **[status_summary.md](status_summary.md)** | **ここまでの状況まとめ**（アーキテクチャ・M0〜M4 + PCB PoC 結果・得られた技術ノウハウ） |
| **[setup_manual.md](setup_manual.md)** | **導入マニュアル**（Windows / Ubuntu 初期構築・SSH トンネル設定・MCP 登録・トラブルシュート） |
| **[operation_manual.md](operation_manual.md)** | **実行マニュアル**（日常の起動〜エージェント指示〜非・常駐クリーンアップ手順） |
| **[agent_rules.md](agent_rules.md)** | **エージェント運用規約**（AI が守るべき安全・座標実測・ウェイト・DRC 検証ルール） |
| **[milestone_plan.md](milestone_plan.md)** | 環境構築マイルストーン計画（M0〜M5） |
| **[checklist.md](checklist.md)** | 実作業チェックリストと検証ログ |
| **[concept.md](concept.md)** | 初期構想メモ |

---

## ディレクトリ構成

- **`win/`**: Windows 側オンデマンド起動ランチャー（`start_easyeda_bridge.bat` / `.ps1` / 設定テンプレート）
- **`ubuntu/`**: Ubuntu 側ツール（疎通診断 `check_bridge.sh` / MCP stdio ブリッジ `easyeda_mcp_stdio.mjs` / CLI ヘルパー `mcp_call.mjs`）
- **`poc_hello_circuit/`**: M4 実証スクリプト（回路図作成〜SCH DRC〜PCB 同期〜配線・GND ベタ〜PCB DRC〜Gerber 出力）
- **`result/`**: 疎通検証ログおよび PoC 出力成果物（回路図 PNG / PCB PNG / BOM / Netlist / Gerber ZIP / CPL）
