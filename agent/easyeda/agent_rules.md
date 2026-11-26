# EasyEDA Pro エージェント運用規約（安全・再現性ルール）

Ubuntu 上の AI エージェントが MCP 経由で Windows 開発 PC 上の EasyEDA Pro を操作する際の共通規約です。  
公式ガイド（[cli-for-ai.md](https://prodocs.easyeda.com/storage/texts/api/guide/cli-for-ai.md)）の制約をベースに、ADX 開発向けの安全策を追加しています。

---

## 1. セッション開始前の必須確認

1. **疎通確認**: 操作を始める前に必ず `doctor`（`easyeda-pro.call` / `path: "doctor"`）を実行し、`connected: true` を確認する。
   - 接続不可の場合は、ユーザーに Windows 側で EasyEDA Pro GUI と `start_easyeda_bridge.bat` を起動してもらう。
2. **対象プロジェクトの明示確認**:
   - **新規作成か既存変更か**、**対象プロジェクト名・保存先**を必ず確認する。
   - PoC / 実験時は本番プロジェクト（既存の ADX クラウド／本番基板データ）に触れず、テスト専用のローカル `.eprj3` プロジェクトに限定する。
3. **既存セッション確認**: `session.list` で現在開いているウィンドウを確認し、ユーザーが作業中の `origin: "user"` セッションを不用意に破壊・終了しない。

---

## 2. `invoke` 実行時の厳守事項

1. **組み込み権限（`extUuid: "eda"`）の明示**:
   - MCP の `invoke` はデフォルトが `eda` だが、ファイル保存やシステム API を呼ぶスクリプトでは `extUuid: "eda"` を意識して実行する。
2. **パス区切りはスラッシュ（`/`）統一**:
   - Windows パスであっても JS コード内では `C:/Users/...` 形式を使う（バックスラッシュのエスケープ事故防止）。
3. **ピン座標は絶対に手計算しない**:
   - 部品を `create()` した直後に、必ず `eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id)` で絶対ピン座標を取得し、その実測値で `SCH_PrimitiveWire.create()` を配線する。
4. **大規模配置はバッチ分割**:
   - 部品配置は 1 回の `invoke` あたり **20〜25 部品以内** に分割し、タイムアウトや中途半端な失敗を防ぐ。
5. **変更後の反映待ち（Settle Wait）**:
   - 部品配置・配線・保存の直後に DRC / ネットリスト / 画像書き出しを行う場合、エディタ内部の更新完了を待つため **5〜10 秒ウェイト** を挟む。
6. **戻り値の真偽を必ず検査**:
   - MCP の `ok: true` は「JS スニペットが例外なく実行終了した」意味にすぎない。API 自体の戻り値・エラー配列・ディスク上の生成物を必ず確認する。

---

## 3. 検証・成果物・終了手順

1. **回路図 DRC の必須実行**:
   - 回路図を構築・変更したら `eda.sch_Document.save()` → ウェイト → `eda.sch_Drc.check(true, false, true)` を実行し、違反件数と内容を報告する。
2. **人間レビュー用エビデンス**:
   - 重要な変更後は PNG スクリーンショットやネットリスト・BOM 等の確認用データを出力し、Windows 画面での目視確認につなげる。
3. **セッションの後片付け**:
   - エージェントが `open` で新規作成したセッションは、作業完了後に `session.close`（`destroy: true`）で閉じる（ユーザーが手動で開いたウィンドウや確認用に残す指示があるものは除く）。
4. **非・常駐運用の維持**:
   - 秘密トークン（`~/.config/easyeda-bridge/token` や `bridge.config.local.ps1`）をログ・ドキュメント・Git コミットへ絶対に出力しない。
   - 作業完了後、ユーザーに Windows 側の `.bat` を `Ctrl+C` で閉じて問題ない旨を案内する。
