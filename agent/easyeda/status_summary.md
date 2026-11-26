# EasyEDA Agent 開発進捗サマリー

最終更新: 2026-11-26（**M0〜M4 回路図＋PCB 一気通貫 PoC 完了** / 導入・実行マニュアル整備済み / M5 準備完了）

---

## 1. 確立したアーキテクチャ（構成 1・非・常駐型）

Windows 開発 PC に常駐サービス（`sshd` 等）を一切置かず、作業時だけ `.bat` を起動して LXD コンテナ上の Ubuntu エージェントと接続する構成を確立・実証しました。

```text
[Ubuntu / LXD コンテナ (AG-G3)]                      [Windows 開発 PC]
  Antigravity (~/.gemini/config/mcp_config.json)       start_easyeda_bridge.bat（作業時のみ起動）
    └─ easyeda_mcp_stdio.mjs (Stdio ⇄ HTTP)             ├─ easyeda-pro --mcp http (127.0.0.1:3030)
         │  ~/.config/easyeda-bridge/token               │    └─ EasyEDA Pro GUI v4.1.105 (半オフライン)
         ▼  POST http://127.0.0.1:3030/mcp               │
       SSH リバーストンネル (-R 3030) ◀──────────────────┴─ ssh -N -R ... AG-G3 (port 2222 / publickey)
```

### 設計上の特徴
- **完全非・常駐**: Windows は標準 OpenSSH クライアント（`ssh.exe`）のみ使用。ファイアウォール受信ポート開放不要。`Ctrl+C` でランチャー起動の `--mcp` プロセス木のみ停止（EasyEDA Pro GUI 本体は落とさない）し、終了時にポート閉鎖を自己確認。
- **デバッグ性重視**: `[1/5]`〜`[5/5]`＋`[CLEANUP]` の段階ログ、Windows ローカル自己疎通テスト、終了時の強制 `pause`、Ubuntu 側の `[1/4]`〜`[4/4]` 診断スクリプト（[ubuntu/check_bridge.sh](ubuntu/check_bridge.sh)）。
- **オフライン待機対応 MCP ブリッジ**: Windows の `.bat` を閉じている間に Antigravity を起動しても落ちず、`.bat` 起動直後から再起動なしで即利用可能（[ubuntu/easyeda_mcp_stdio.mjs](ubuntu/easyeda_mcp_stdio.mjs)）。
- **機密分離**: Bearer トークンは Windows 側 `bridge.config.local.ps1`（`.gitignore` 対象）と Ubuntu 側 `~/.config/easyeda-bridge/token`（権限 `600`）に分離し、Git や `mcp_config.json` には一切記載しない。

---

## 2. マイルストーン達成状況

| ID | 内容 | 状態 | 根拠ログ / 成果物 |
| :--- | :--- | :--- | :--- |
| **M0** | Windows 側 EasyEDA Pro デスクトップ版導入 | **完了** | `4.1.105.0339a6cd`・半オフラインモード・`doctor` `connected: true`（[checklist.md](checklist.md)） |
| **M1** | Windows 単体 CLI 操作 PoC | **完了** | `open` / `invoke` / `doc api` / `session list` / `session close --destroy` 全成功（[checklist.md](checklist.md)） |
| **M2** | 非・常駐 `.bat` ランチャー & LAN 疎通確認 | **完了** | トンネル開通・無認証 `401`・認証 `200`・ツール 3 種検出・停止時ポート閉鎖確認（[result/20261126_1457.md](result/20261126_1457.md) / [result/20261126_1502.md](result/20261126_1502.md)） |
| **M3** | Ubuntu エージェント MCP 統合 & 運用ルール整備 | **完了** *(UI 目視確認のみ残)* | `~/.gemini/config/mcp_config.json` 登録、`easyeda_mcp_stdio.mjs` ライブ＆待機テスト通過、`doctor`/`session.list`/`doc.api` 疎通確認、[agent_rules.md](agent_rules.md) 作成 |
| **M4** | 「Hello Circuit」自動構築 PoC（回路図＋PCB） | **完了** | 回路図目視確認済み・SCH DRC 0・PCB DRC 0・未配線 0・PNG / BOM / Netlist / Gerber / CPL すべて出力＆検品完了 |
| **M5** | ADX 設計ワークフロー応用 | 未着手 | M4 目視確認後に展開 |

---

## 3. M4「Hello Circuit」PoC 実証結果

- **対象プロジェクト**: `Agent_Hello_Circuit`（`.eprj3` / `C:\Users\User\Documents\EasyEDA-Pro\projects` 配下）
- **実行スクリプト**: [poc_hello_circuit/step1_create_and_place.js](poc_hello_circuit/step1_create_and_place.js) → [poc_hello_circuit/step2_wire_drc_export.js](poc_hello_circuit/step2_wire_drc_export.js)
- **構築した回路**: 分圧＋RC ローパスフィルタ（`R1`・`R2`・`C1`）＋電源インジケータ LED（`R3`・`LED1`）
- **選定部品（すべてアクティブな JLCPCB Basic Part）**:
  - `R1`, `R3`: `1kΩ` 0603 (`0603WAF1001T5E` / LCSC `C21190`)
  - `R2`: `10kΩ` 0603 (`0603WAF1002T5E` / LCSC `C25804`)
  - `C1`: `100nF` 50V X7R 0603 (`CC0603KRX7R9BB104` / LCSC `C14663`)
  - `LED1`: Red LED 0603 (`KT-0603R` / LCSC `C2286`, Pin1=`A` 上 / Pin2=`K` 下を実測確認)
- **DRC 結果**: `SCH_Drc.check(true, false, false)` = **`true`**、`check(true, false, true)` = **`[]`（エラー・警告 0 件）**
- **ネットリスト接続検証結果**（[result/hello_circuit_netlist.enet](result/hello_circuit_netlist.enet)）:
  - `VCC`: `R1.1`, `R3.1`
  - `$1N2`（分圧＋フィルタ節点）: `R1.2`, `R2.1`, `C1.1`
  - `$1N3`（LED アノード）: `R3.2`, `LED1.1`
  - `GND`: `R2.2`, `C1.2`, `LED1.2`
- **出力エビデンス（回路図）**:
  - 回路図 PNG: [result/hello_circuit_sch.png](result/hello_circuit_sch.png)（目視確認済み：重複なし・接合点ドット正常・LED 向き正常）
  - BOM CSV: [result/hello_circuit_bom.csv](result/hello_circuit_bom.csv)
  - ネットリスト: [result/hello_circuit_netlist.enet](result/hello_circuit_netlist.enet)
  - 実行ログ JSON: [result/hello_circuit_step1.json](result/hello_circuit_step1.json) / [result/hello_circuit_step2.json](result/hello_circuit_step2.json)

### M4 拡張：PCB 自動生成〜製造データ出力 PoC 実証結果（案 A）
- **実行スクリプト**: [poc_hello_circuit/step3_pcb_import_place.js](poc_hello_circuit/step3_pcb_import_place.js) → [poc_hello_circuit/step4_pcb_route_drc_export.js](poc_hello_circuit/step4_pcb_route_drc_export.js)
- **基板外形**: `1000 × 800 mil`（`25.4 × 20.32 mm`）2 層テスト基板（`BoardOutline` レイヤ 11）
- **配置・配線・ベタ**:
  - `pcb_Document.importChanges(undefined, true)` で確認ダイアログ無しに回路図から自動同期
  - 5 部品を回路図トポロジ通りに配置し、`getAllPinsByPrimitiveId()` の実測パッド座標・ネット名で向きを自動反転調整
  - トップ層 15 mil 配線（13 セグメント・交差ゼロ・ビア不要）＋トップ層 GND ベタ塗り（`GND_TOP`・外形から 20 mil 内側）＋`rebuildAllCopperRegions()`
- **PCB DRC & ラッツネスト結果**:
  - `pcb_Drc.check(true, false, false)` = **`true`**、`check(true, false, true)` = **`[]`（エラー・警告 0 件）**
  - `pcb_Document.getRatlines()` = **`total: 0`（未配線 0 本）**
- **出力エビデンス（PCB / 製造データ）**:
  - PCB プレビュー PNG: [result/hello_circuit_pcb.png](result/hello_circuit_pcb.png)（目視確認済み：外形・15 mil 配線・GND ベタ・シルクすべて正常・重複なし）
  - Gerber ZIP: [result/hello_circuit_gerber.zip](result/hello_circuit_gerber.zip)（GTL/GBL/GTO/GBO/GTS/GBS/GTP/GKO/GDL/GDD/FlyingProbe 完備）
  - Pick & Place（CPL）: [result/hello_circuit_cpl.xlsx](result/hello_circuit_cpl.xlsx)
  - 実行ログ JSON: [result/hello_circuit_step3_pcb.json](result/hello_circuit_step3_pcb.json) / [result/hello_circuit_step4_pcb.json](result/hello_circuit_step4_pcb.json)

### 今回得られた技術ノウハウ（M5 に活かす知見）
1. `LIB_Device.getByLcscIds(cid)` は非全オフライン環境でも安定して正確な LCSC 部品情報（廃番判定や Basic/Extended 区分含む）を返せる（例：`C72043` が廃番・Extended であることも自動検知し、`C2286` Basic Part へ切替できた）。
2. `DMT_Project.createProject()` 自体がデフォルトで `Board1` を生成するため、直後に `createBoard()` を呼ぶと `Board2` が増える（不要ボードは `deleteBoard()` で整理可能、または既存 `Board1` をそのまま使うのがスマート）。
3. `getAllPinsByPrimitiveId()` の座標には 90° 回転時の浮動小数点誤差（`419.99999999999994` 等）が混じるため、小数点第 2 位丸め（`Math.round(v * 100) / 100`）で半グリッド `.5` を保ちつつ直交性を安定化できた。
4. T 字分岐を必ず端点同士が出会うセグメントに分割して配線することで、接合点ドットとネット接続が 100% 確実に形成された。
5. `SYS_FileSystem.getLibrariesPaths()` は完全オフライン専用 API のため、半オフライン環境では呼ばないこと。
6. PCB 側では `pcb_Document.importChanges(undefined, true)` の `force = true` により GUI ダイアログをスキップして完全自動化でき、`pcb_PrimitiveComponent.getAllPinsByPrimitiveId()` で実測したパッド座標・ネット名から向き補正＋配線すると一発で未配線 0・DRC 0 を達成できる。
7. `pcb_ManufactureData.getPickAndPlaceFile()` のデフォルト出力は `.xlsx` 形式。

---

## 4. 検証経緯の記録（デバッグ履歴）

1. [result/20261126_1450.md](result/20261126_1450.md): Windows ローカル MCP（`/mcp`・無認証 `401`・認証 `200`・4ms 応答）および非・常駐クリーンアップ成功。SSH 先設定の誤記を検出・修正。
2. [result/20261126_1453.md](result/20261126_1453.md): `$SSH_PORT = 22` が `~/.ssh/config` の `AG-G3` 用ポート（2222）を上書きしていたことを特定・修正。
3. [result/20261126_1457.md](result/20261126_1457.md): LXD コンテナ直通トンネル開通、Ubuntu → Windows 到達（`401`）、`Ctrl+C` によるポート 3030 閉鎖を確認。
4. [result/20261126_1502.md](result/20261126_1502.md): `~/.config/easyeda-bridge/token` による認証付き `initialize` / `tools/list` まで全 4 項目 `PASS`。

---

## 5. 作成済みファイル一覧

- ナビゲーション・マニュアル: [README.md](README.md) / [setup_manual.md](setup_manual.md) / [operation_manual.md](operation_manual.md)
- 計画・規約・サマリー: [concept.md](concept.md) / [milestone_plan.md](milestone_plan.md) / [checklist.md](checklist.md) / [agent_rules.md](agent_rules.md) / [status_summary.md](status_summary.md)
- Windows 側ランチャー: [win/start_easyeda_bridge.bat](win/start_easyeda_bridge.bat) / [win/start_easyeda_bridge.ps1](win/start_easyeda_bridge.ps1) / [win/bridge.config.example.ps1](win/bridge.config.example.ps1) / [win/.gitignore](win/.gitignore)
- Ubuntu 側ツール: [ubuntu/check_bridge.sh](ubuntu/check_bridge.sh) / [ubuntu/easyeda_mcp_stdio.mjs](ubuntu/easyeda_mcp_stdio.mjs) / [ubuntu/mcp_call.mjs](ubuntu/mcp_call.mjs)
- M4 PoC スクリプト: [poc_hello_circuit/step1_create_and_place.js](poc_hello_circuit/step1_create_and_place.js) / [poc_hello_circuit/step2_wire_drc_export.js](poc_hello_circuit/step2_wire_drc_export.js) / [poc_hello_circuit/step3_pcb_import_place.js](poc_hello_circuit/step3_pcb_import_place.js) / [poc_hello_circuit/step4_pcb_route_drc_export.js](poc_hello_circuit/step4_pcb_route_drc_export.js)
- 実行結果・成果物: [result/](result/)
