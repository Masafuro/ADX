<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# WU-4: 市販安価ドングル ＆ WebSerial (PWA) 直結実証

本サンドボックスは、MR32 仕様における **市販の安価な USB-RS485 ドングル（CH340/CP2102 等）と WebSerial API (PWA) を用いたブラウザ直結 OTW ファームウェア更新** の実証環境です。

---

## 🚀 実装場所：プロジェクト公式 `docs/flasher/` (GitHub Pages 対応)

本ツールは、ローカル環境だけでなく **GitHub Pages（`https://.../ADX/flasher/`）** 上で世界中どこからでも即座にブラウザから利用できるよう、プロジェクト公式の [`docs/flasher/`](../../../../docs/flasher/) に完全実装・統合されています：

```text
docs/flasher/
├── index.html        # リッチ・サイバーコンソール UI (HTML5)
├── style.css         # バニラ CSS (ダークグラスモーフィズム・ネオングロー)
├── preset_app.js     # M4 実証済み LED 交互点滅アプリ (12KB) 内蔵データ
├── app.js            # MR32 WebSerial 通信エンジン ＆ OTW ストリーミング
├── manifest.json     # PWA マニフェスト (Android / PC アプリ化)
├── sw.js             # サービスワーカー (オフライン対応)
└── icon.svg          # PWA ベクターアイコン
```

---

## 🛠️ ハードウェア接続

- **ADX Core-D**:
  - H2: 2-3 ショート (連動)
  - H3: 1-2 ショート (内部 20MHz)
  - H4: OPEN
  - マイコン内ファーム: M4 ブートローダー（`m4_bootloader.hex`）
- **USB-RS485 ドングル**:
  - 市販 CH340 / CP2102 ドングル（COM22 @ 115,200 bps）

---

## 🌐 実行手順

### 方法 A: GitHub Pages（推奨・完全ブラウザ完結）
GitHub に push 後、以下の URL を Chrome / Edge で開くだけです：
```text
https://<GitHubユーザー名>.github.io/ADX/flasher/
```

### 方法 B: ローカルファイルから即座にテスト
PC の Chrome / Edge ブラウザで、以下のローカル HTML を直接開くだけでも動作します：
```powershell
# ブラウザで直接開く
start chrome docs/flasher/index.html
```

---

## 🎯 テスト手順

1. **[シリアルポートを選択して接続]** をクリックし、市販ドングルのポート（`COM22` 等）を選択。
2. **[📡 Send Ping (0x10)]** をクリックし、Core-D からデバイス情報（ATtiny1616）が返信されることを確認。
3. **[🚀 プリセットをワンクリックロード]** をクリック。
   - M4 で実証された 12KB ファームウェア（LED 交互点滅スケッチ）が即座にスロットにセットされます。
4. **[🔥 12KB フル OTW 書込開始]** をクリック！
   - プログレスバーが滑らかに進み、約 5〜6 秒で 12KB 全域が書き換えられます。
   - 完了後、自動的にユーザーアプリが起動します！
5. **[基板の確認]**:
   - 赤 LED（PB2）と 白 LED（PB3）が高速交互点滅を開始することを確認！

---

## 📝 結果の記録

検証結果は以下の公式レポートに記録します：  
📄 **[`firmware/sandbox/records/WU4_web_serial_report.md`](../../records/WU4_web_serial_report.md)**
