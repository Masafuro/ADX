/*
 * ADX MR32 WebSerial Flasher & Diagnostic Engine
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 */

// =========================================================================
// MR32 Protocol Constants & Specifications
// =========================================================================
const MR32_SYNC_BYTE       = 0x55;
const MR32_MAGIC_BYTE      = 0xAD;
const MR32_FRAME_LEN       = 32;
const MR32_PAYLOAD_LEN     = 24;

const CMD_BOOT_PING        = 0x10;
const CMD_BOOT_WRITE_CHUNK = 0x11;
const CMD_BOOT_READ_CHUNK  = 0x12;
const CMD_BOOT_CRC_CHECK   = 0x13;
const CMD_BOOT_APP_EXEC    = 0x14;

const STATUS_OK            = 0x00;
const STATUS_ERR_PARAM     = 0x03;
const STATUS_PAGE_DONE     = 0x10;

const APP_START_PAGE       = 64;
const APP_TOTAL_PAGES      = 192; // Pages 64..255 (12KB)
const FLASH_PAGE_SIZE      = 64;
const CHUNK_SIZE           = 16;
const CHUNKS_PER_PAGE      = 4;

// =========================================================================
// CRC-16-CCITT Engine (Polynomial 0x1021, Initial 0xFFFF)
// =========================================================================
function calculateCRC16(data, initVal = 0xFFFF) {
  let crc = initVal;
  for (let i = 0; i < data.length; i++) {
    crc ^= (data[i] << 8);
    for (let j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = ((crc << 1) ^ 0x1021) & 0xFFFF;
      } else {
        crc = (crc << 1) & 0xFFFF;
      }
    }
  }
  return crc;
}

// Build 32-byte MR32 Fixed-length Frame
function buildMr32Frame(dstId, srcId, cmd, seqNum, payload24 = null) {
  const frame = new Uint8Array(MR32_FRAME_LEN);
  frame[0] = MR32_SYNC_BYTE;
  frame[1] = MR32_MAGIC_BYTE;
  frame[2] = dstId & 0xFF;
  frame[3] = srcId & 0xFF;
  frame[4] = cmd & 0xFF;
  frame[5] = seqNum & 0xFF;

  if (payload24) {
    const copyLen = Math.min(payload24.length, MR32_PAYLOAD_LEN);
    frame.set(payload24.subarray(0, copyLen), 6);
  }

  // Calculate CRC over body (bytes 2..29, length 28)
  const crc = calculateCRC16(frame.subarray(2, 30));
  frame[30] = crc & 0xFF;
  frame[31] = (crc >> 8) & 0xFF;

  return frame;
}

// Parse and Validate 32-byte MR32 Frame
function parseMr32Frame(frame) {
  if (frame.length !== MR32_FRAME_LEN) {
    return { valid: false, error: `Invalid length: ${frame.length} != 32` };
  }
  if (frame[0] !== MR32_SYNC_BYTE || frame[1] !== MR32_MAGIC_BYTE) {
    return { valid: false, error: `Magic error: [0x${frame[0].toString(16)}, 0x${frame[1].toString(16)}]` };
  }

  const expectedCrc = calculateCRC16(frame.subarray(2, 30));
  const receivedCrc = frame[30] | (frame[31] << 8);
  if (expectedCrc !== receivedCrc) {
    return { valid: false, error: `CRC mismatch: expected 0x${expectedCrc.toString(16)}, got 0x${receivedCrc.toString(16)}` };
  }

  return {
    valid: true,
    sync: frame[0],
    magic: frame[1],
    dstId: frame[2],
    srcId: frame[3],
    cmd: frame[4],
    seqNum: frame[5],
    payload: frame.subarray(6, 30),
    crc16: receivedCrc
  };
}

// Helper: Format bytes to HEX string
function toHexStr(bytes) {
  return Array.from(bytes).map(b => b.toString(16).padStart(2, '0')).join(' ');
}

// =========================================================================
// WebSerial Communication Controller
// =========================================================================
class SerialManager {
  constructor() {
    this.port = null;
    this.reader = null;
    this.writer = null;
    this.isConnected = false;
    this.isTransferring = false;
    this.rxBuffer = []; // FIFO buffer for incoming bytes
    this.readingActive = false;
    this.readLoopPromise = null;
  }

  async connect() {
    if (!('serial' in navigator)) {
      throw new Error('お使いのブラウザは WebSerial API に対応していません。Chrome または Edge を使用してください。');
    }

    this.port = await navigator.serial.requestPort();
    await this.port.open({
      baudRate: 115200,
      dataBits: 8,
      stopBits: 1,
      parity: 'none',
      bufferSize: 4096
    });

    this.isConnected = true;
    this.writer = this.port.writable.getWriter();
    this.readingActive = true;
    this.rxBuffer = [];

    // Start background stream ingestion loop
    this.readLoopPromise = this._startReadLoop();
    return true;
  }

  async _startReadLoop() {
    while (this.port && this.readingActive) {
      try {
        this.reader = this.port.readable.getReader();
        while (this.readingActive) {
          const { value, done } = await this.reader.read();
          if (done) break;
          if (value && value.length > 0) {
            for (let i = 0; i < value.length; i++) {
              this.rxBuffer.push(value[i]);
            }
          }
        }
      } catch (err) {
        if (this.readingActive) {
          console.warn('WebSerial Read Loop Warning:', err);
        }
      } finally {
        if (this.reader) {
          try { this.reader.releaseLock(); } catch (e) {}
          this.reader = null;
        }
      }
    }
  }

  async disconnect() {
    this.isConnected = false;
    this.readingActive = false;

    if (this.reader) {
      try { await this.reader.cancel(); } catch (e) {}
    }
    if (this.readLoopPromise) {
      try { await this.readLoopPromise; } catch (e) {}
      this.readLoopPromise = null;
    }
    if (this.writer) {
      try { await this.writer.close(); } catch (e) {}
      try { this.writer.releaseLock(); } catch (e) {}
      this.writer = null;
    }
    if (this.port) {
      try { await this.port.close(); } catch (e) {}
      this.port = null;
    }
    this.rxBuffer = [];
  }

  clearRx() {
    this.rxBuffer = [];
  }

  // Send a 32-byte frame and wait for 32-byte response with Header Hunting
  async sendAndReceive(txFrame, timeoutMs = 200) {
    if (!this.isConnected || !this.writer) {
      throw new Error('シリアルポートが接続されていません。');
    }

    // 1. Flush any stale bytes before transmission (like pyserial reset_input_buffer)
    this.clearRx();

    const tStart = performance.now();

    // 2. Transmit frame
    await this.writer.write(txFrame);

    // 3. Wait for 32-byte response frame with SYNC(0x55) and MAGIC(0xAD) header hunting
    const deadline = performance.now() + timeoutMs;
    let foundFrame = null;

    while (performance.now() < deadline) {
      if (this.rxBuffer.length >= 2) {
        let syncIdx = -1;
        for (let i = 0; i <= this.rxBuffer.length - 2; i++) {
          if (this.rxBuffer[i] === MR32_SYNC_BYTE && this.rxBuffer[i + 1] === MR32_MAGIC_BYTE) {
            syncIdx = i;
            break;
          }
        }

        if (syncIdx > 0) {
          // Drop garbage noise bytes before SYNC
          this.rxBuffer.splice(0, syncIdx);
        }

        if (syncIdx >= 0 && this.rxBuffer.length >= MR32_FRAME_LEN) {
          // Complete 32-byte MR32 frame extracted!
          foundFrame = new Uint8Array(this.rxBuffer.splice(0, MR32_FRAME_LEN));
          break;
        }
      }

      await new Promise(r => setTimeout(r, 2)); // 2ms polling sleep
    }

    const tEnd = performance.now();
    const rtt = tEnd - tStart;

    if (foundFrame) {
      return { ok: true, data: foundFrame, rtt };
    }

    // If timeout, return whatever raw bytes were captured
    const partial = new Uint8Array(this.rxBuffer);
    return { ok: false, data: partial, rtt, timeout: true };
  }
}

// Global Manager Instance
const serialMgr = new SerialManager();

// =========================================================================
// UI Controller & State
// =========================================================================
let loadedFirmwareData = null; // 12,288 Bytes (192 pages)
let loadedFirmwareName = "";

// DOM Elements
const statusDot = document.getElementById('statusDot');
const statusText = document.getElementById('statusText');
const btnConnect = document.getElementById('btnConnect');
const btnDisconnect = document.getElementById('btnDisconnect');
const targetNodeInput = document.getElementById('targetNodeId');

const btnPing = document.getElementById('btnPing');
const btnProtectTest = document.getElementById('btnProtectTest');
const btnPresetM4 = document.getElementById('btnPresetM4');
const btnFlashOTW = document.getElementById('btnFlashOTW');
const btnLaunchApp = document.getElementById('btnLaunchApp');
const fileInput = document.getElementById('fileInput');
const dropZone = document.getElementById('dropZone');

const fwInfoBox = document.getElementById('fwInfoBox');
const fwFileName = document.getElementById('fwFileName');
const fwFileSize = document.getElementById('fwFileSize');
const fwCrcBadge = document.getElementById('fwCrcBadge');

const progressBar = document.getElementById('progressBar');
const progressPercent = document.getElementById('progressPercent');
const progressSubText = document.getElementById('progressSubText');
const metricSpeed = document.getElementById('metricSpeed');
const metricElapsed = document.getElementById('metricElapsed');
const metricAvgRtt = document.getElementById('metricAvgRtt');
const metricCurrentPage = document.getElementById('metricCurrentPage');

const consoleOutput = document.getElementById('consoleOutput');
const btnClearLog = document.getElementById('btnClearLog');

// Logging utility
function log(msg, type = 'info') {
  const now = new Date();
  const timeStr = `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}.${now.getMilliseconds().toString().padStart(3, '0')}`;
  
  const entry = document.createElement('div');
  entry.className = 'log-entry';
  entry.innerHTML = `<span class="log-time">[${timeStr}]</span> <span class="log-${type}">${msg}</span>`;
  consoleOutput.appendChild(entry);
  consoleOutput.scrollTop = consoleOutput.scrollHeight;
}

// Log MR32 Packets
function logPacket(direction, frame, rtt = 0) {
  const cmdNames = {
    0x10: 'PING',
    0x11: 'WRITE_CHUNK',
    0x12: 'READ_CHUNK',
    0x13: 'CRC_CHECK',
    0x14: 'APP_EXEC'
  };
  const cmd = frame[4];
  const cmdName = cmdNames[cmd] || `CMD:0x${cmd.toString(16)}`;
  const hex = toHexStr(frame);
  const rttStr = rtt > 0 ? ` (${rtt.toFixed(1)}ms)` : '';

  if (direction === 'TX') {
    log(`TX ▶ [${cmdName}] ${hex}`, 'tx');
  } else {
    log(`RX ◀ [${cmdName}] ${hex}${rttStr}`, 'rx');
  }
}

// Update Connection UI
function updateConnectionUI(connected, portInfo = '') {
  if (connected) {
    statusDot.className = 'status-dot connected';
    statusText.textContent = `CONNECTED (115.2k 8N1)`;
    btnConnect.disabled = true;
    btnDisconnect.disabled = false;
    btnPing.disabled = false;
    btnProtectTest.disabled = false;
    btnLaunchApp.disabled = false;
    if (loadedFirmwareData) btnFlashOTW.disabled = false;
    log(`RS-485 ポートに正常接続しました (@ 115,200 bps 8N1)`, 'success');
  } else {
    statusDot.className = 'status-dot';
    statusText.textContent = 'DISCONNECTED';
    btnConnect.disabled = false;
    btnDisconnect.disabled = true;
    btnPing.disabled = true;
    btnProtectTest.disabled = true;
    btnFlashOTW.disabled = true;
    btnLaunchApp.disabled = true;
    log(`シリアルポートを切断しました。`, 'info');
  }
}

// =========================================================================
// Firmware Loader (Preset & Custom File)
// =========================================================================
function setFirmware(data, name) {
  // Pad or slice to exactly 12,288 Bytes (192 pages * 64B)
  const targetLen = APP_TOTAL_PAGES * FLASH_PAGE_SIZE;
  const padded = new Uint8Array(targetLen);
  padded.fill(0xFF);
  padded.set(data.subarray(0, Math.min(data.length, targetLen)));

  loadedFirmwareData = padded;
  loadedFirmwareName = name;

  const totalCrc = calculateCRC16(loadedFirmwareData);
  fwFileName.textContent = name;
  fwFileSize.textContent = `${loadedFirmwareData.length} Bytes (192 Pages / 12KB)`;
  fwCrcBadge.textContent = `CRC: 0x${totalCrc.toString(16).toUpperCase().padStart(4, '0')}`;
  fwInfoBox.style.display = 'flex';

  if (serialMgr.isConnected) {
    btnFlashOTW.disabled = false;
  }
  log(`ファームウェアをロードしました: ${name} (${loadedFirmwareData.length} Bytes, CRC: 0x${totalCrc.toString(16).toUpperCase()})`, 'info');
}

function loadPreset(b64Str, name) {
  if (!b64Str) {
    log(`プリセットデータ [${name}] が見つかりません。`, 'error');
    return;
  }
  const binaryStr = atob(b64Str);
  const bytes = new Uint8Array(binaryStr.length);
  for (let i = 0; i < binaryStr.length; i++) {
    bytes[i] = binaryStr.charCodeAt(i);
  }
  setFirmware(bytes, name);
}

// Preset Buttons
btnPresetM4.addEventListener('click', () => {
  loadPreset(window.BUILTIN_M4_APP_BASE64 || BUILTIN_M4_APP_BASE64, 'M4 Alternating LED Blink App (app_12k.bin)');
});

const btnSample1 = document.getElementById('btnSample1');
if (btnSample1) {
  btnSample1.addEventListener('click', () => {
    loadPreset(window.SAMPLE1_RED_SOS_BASE64 || SAMPLE1_RED_SOS_BASE64, 'Sample 1: Red LED Morse SOS (sample1_red_sos.bin)');
  });
}

const btnSample2 = document.getElementById('btnSample2');
if (btnSample2) {
  btnSample2.addEventListener('click', () => {
    loadPreset(window.SAMPLE2_WHITE_STROBE_BASE64 || SAMPLE2_WHITE_STROBE_BASE64, 'Sample 2: White LED Strobe (sample2_white_strobe.bin)');
  });
}

const btnSample3 = document.getElementById('btnSample3');
if (btnSample3) {
  btnSample3.addEventListener('click', () => {
    loadPreset(window.SAMPLE3_SMART_ECHO_BASE64 || SAMPLE3_SMART_ECHO_BASE64, 'Sample 3: Smart Auto-Reboot (sample3_smart_echo.bin)');
  });
}

// File Upload / Drop Handling
fileInput.addEventListener('change', (e) => {
  const file = e.target.files[0];
  if (file) handleFile(file);
});

dropZone.addEventListener('click', () => fileInput.click());
dropZone.addEventListener('dragover', (e) => {
  e.preventDefault();
  dropZone.classList.add('dragover');
});
dropZone.addEventListener('dragleave', () => dropZone.classList.remove('dragover'));
dropZone.addEventListener('drop', (e) => {
  e.preventDefault();
  dropZone.classList.remove('dragover');
  if (e.dataTransfer.files.length > 0) {
    handleFile(e.dataTransfer.files[0]);
  }
});

// Intel HEX Format Parser (Auto-maps to 0x1000..0x3FFF)
function parseIntelHex(hexText) {
  const lines = hexText.split(/\r?\n/);
  const targetLen = APP_TOTAL_PAGES * FLASH_PAGE_SIZE; // 12,288 Bytes
  const appData = new Uint8Array(targetLen);
  appData.fill(0xFF);

  let upperAddr = 0;
  let recordCount = 0;

  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed.startsWith(':')) continue;

    const byteCount = parseInt(trimmed.substr(1, 2), 16);
    const addr = parseInt(trimmed.substr(3, 4), 16);
    const recordType = parseInt(trimmed.substr(7, 2), 16);
    const dataStr = trimmed.substr(9, byteCount * 2);

    if (recordType === 0x00) { // Data Record
      const fullAddr = upperAddr + addr;
      let offset = -1;
      if (fullAddr >= 0x1000 && fullAddr < 0x4000) {
        offset = fullAddr - 0x1000;
      } else if (fullAddr < 0x3000) {
        offset = fullAddr;
      }

      if (offset >= 0 && offset + byteCount <= appData.length) {
        for (let i = 0; i < byteCount; i++) {
          appData[offset + i] = parseInt(dataStr.substr(i * 2, 2), 16);
        }
        recordCount++;
      }
    } else if (recordType === 0x02) { // Extended Segment
      upperAddr = parseInt(dataStr, 16) << 4;
    } else if (recordType === 0x04) { // Extended Linear
      upperAddr = parseInt(dataStr, 16) << 16;
    } else if (recordType === 0x01) { // EOF
      break;
    }
  }

  if (recordCount === 0) {
    throw new Error('有効な Intel HEX データレコードが見つかりませんでした。');
  }
  return appData;
}

function handleFile(file) {
  const isHex = file.name.toLowerCase().endsWith('.hex');
  const reader = new FileReader();

  if (isHex) {
    reader.onload = (e) => {
      try {
        const text = e.target.result;
        const parsed = parseIntelHex(text);
        setFirmware(parsed, file.name);
      } catch (err) {
        log(`HEXパースエラー: ${err.message}`, 'error');
      }
    };
    reader.readAsText(file);
  } else {
    reader.onload = (e) => {
      const raw = new Uint8Array(e.target.result);
      setFirmware(raw, file.name);
    };
    reader.readAsArrayBuffer(file);
  }
}

// =========================================================================
// Actions (Connect, Ping, Protection, OTW Flash, Launch)
// =========================================================================

// Connect
btnConnect.addEventListener('click', async () => {
  try {
    await serialMgr.connect();
    updateConnectionUI(true);
  } catch (err) {
    log(`接続失敗: ${err.message}`, 'error');
  }
});

// Disconnect
btnDisconnect.addEventListener('click', async () => {
  await serialMgr.disconnect();
  updateConnectionUI(false);
});

// Ping
btnPing.addEventListener('click', async () => {
  const nodeId = parseInt(targetNodeInput.value, 16) || 0x01;
  log(`Node 0x${nodeId.toString(16).padStart(2, '0')} へ Ping (0x10) を送信中...`, 'info');

  const frame = buildMr32Frame(nodeId, 0x00, CMD_BOOT_PING, 1);
  logPacket('TX', frame);

  const res = await serialMgr.sendAndReceive(frame, 250);
  if (!res.ok) {
    const rxLen = res.data ? res.data.length : 0;
    const rawHex = rxLen > 0 ? ` (受信バイト列: ${toHexStr(res.data)})` : ' (受信バイト数: 0)';
    log(`Ping タイムアウト: 応答がありません${rawHex}`, 'warn');
    return;
  }

  logPacket('RX', res.data, res.rtt);
  const parsed = parseMr32Frame(res.data);
  if (!parsed.valid) {
    log(`パケット検証エラー: ${parsed.error}`, 'error');
    return;
  }

  const p = parsed.payload;
  const mcuId = (p[1] << 8) | p[2];
  const flashKb = p[3];
  const pageB = p[4];
  log(`★ Ping 成功! RTT=${res.rtt.toFixed(1)}ms | MCU: 0x${mcuId.toString(16).toUpperCase()} (ATtiny1616), Flash: ${flashKb}KB, Page: ${pageB}B`, 'success');
});

// Protection Test
btnProtectTest.addEventListener('click', async () => {
  const nodeId = parseInt(targetNodeInput.value, 16) || 0x01;
  log(`自爆防止ガード検証: 保護領域 Page 0 への書込を試行中...`, 'info');

  const payload = new Uint8Array(24);
  // req_page = 0, chunk = 0
  payload[0] = 0x00;
  payload[1] = 0x00;
  payload[2] = 0x00;

  const frame = buildMr32Frame(nodeId, 0x00, CMD_BOOT_WRITE_CHUNK, 1, payload);
  logPacket('TX', frame);

  const res = await serialMgr.sendAndReceive(frame, 250);
  if (!res.ok) {
    const rxLen = res.data ? res.data.length : 0;
    const rawHex = rxLen > 0 ? ` (受信: ${toHexStr(res.data)})` : ' (受信: 0B)';
    log(`保護テストタイムアウト${rawHex}`, 'warn');
    return;
  }

  logPacket('RX', res.data, res.rtt);
  const parsed = parseMr32Frame(res.data);
  if (!parsed.valid) {
    log(`パケット検証エラー: ${parsed.error}`, 'error');
    return;
  }

  const status = parsed.payload[0];
  if (status === STATUS_ERR_PARAM) {
    log(`★ 自爆防止ガード発動確認! Page 0 書込は STATUS_ERR_PARAM (0x03) で安全に拒絶されました (RTT=${res.rtt.toFixed(1)}ms)`, 'success');
  } else {
    log(`[WARN] 予期しないステータス: 0x${status.toString(16)}`, 'warn');
  }
});

// Launch User Application
btnLaunchApp.addEventListener('click', async () => {
  const nodeId = parseInt(targetNodeInput.value, 16) || 0x01;
  log(`ユーザーアプリ起動指示 (0x14) を送信中...`, 'info');

  const frame = buildMr32Frame(nodeId, 0x00, CMD_BOOT_APP_EXEC, 1);
  logPacket('TX', frame);

  const res = await serialMgr.sendAndReceive(frame, 250);
  if (!res.ok) {
    const rxLen = res.data ? res.data.length : 0;
    const rawHex = rxLen > 0 ? ` (受信: ${toHexStr(res.data)})` : ' (受信: 0B)';
    log(`起動指示タイムアウト${rawHex}`, 'warn');
    return;
  }

  logPacket('RX', res.data, res.rtt);
  log(`★ ブートローダーが起動コマンドを受理しました! 0x1000 へジャンプします。`, 'success');
  log(`>>> 基板の赤LED(PB2)と白LED(PB3)の高速交互点滅を確認してください! <<<`, 'success');
});

// 12KB High-Speed Full OTW Flasher
btnFlashOTW.addEventListener('click', async () => {
  if (!loadedFirmwareData || serialMgr.isTransferring) return;

  const nodeId = parseInt(targetNodeInput.value, 16) || 0x01;
  serialMgr.isTransferring = true;
  statusDot.className = 'status-dot flashing';
  statusText.textContent = 'FLASHING 12KB OTW...';
  btnFlashOTW.disabled = true;
  btnPing.disabled = true;
  btnProtectTest.disabled = true;

  log(`=== 12KB フル OTW ファームウェア更新を開始 (Pages 64..255) ===`, 'info');

  const tStart = performance.now();
  const pageRtts = [];

  try {
    for (let pIdx = 0; pIdx < APP_TOTAL_PAGES; pIdx++) {
      const pageNo = APP_START_PAGE + pIdx;
      const pageData = loadedFirmwareData.subarray(pIdx * FLASH_PAGE_SIZE, (pIdx + 1) * FLASH_PAGE_SIZE);
      const expectedPageCrc = calculateCRC16(pageData);

      const tPageStart = performance.now();

      // Send 4 chunks
      for (let c = 0; c < CHUNKS_PER_PAGE; c++) {
        const cData = pageData.subarray(c * CHUNK_SIZE, (c + 1) * CHUNK_SIZE);
        const payload = new Uint8Array(24);
        payload[0] = pageNo & 0xFF;
        payload[1] = (pageNo >> 8) & 0xFF;
        payload[2] = c;
        payload.set(cData, 3);

        const frame = buildMr32Frame(nodeId, 0x00, CMD_BOOT_WRITE_CHUNK, c, payload);
        const res = await serialMgr.sendAndReceive(frame, 200);

        if (!res.ok) {
          const rxLen = res.data ? res.data.length : 0;
          const rawHex = rxLen > 0 ? ` [生データ: ${toHexStr(res.data)}]` : ' [受信0バイト]';
          throw new Error(`Page ${pageNo} Chunk ${c} 書込タイムアウト${rawHex}`);
        }

        if (c === 3) {
          const parsed = parseMr32Frame(res.data);
          if (!parsed.valid) throw new Error(`Page ${pageNo} 応答破損: ${parsed.error}`);

          const status = parsed.payload[0];
          const flashCrc = parsed.payload[5] | (parsed.payload[6] << 8);

          if (status !== STATUS_PAGE_DONE) {
            throw new Error(`Page ${pageNo} コミット失敗: status 0x${status.toString(16)}`);
          }
          if (flashCrc !== expectedPageCrc) {
            throw new Error(`Page ${pageNo} CRC不一致: Flash 0x${flashCrc.toString(16)} != 期待値 0x${expectedPageCrc.toString(16)}`);
          }
        }
      }

      const tPageEnd = performance.now();
      const pageRtt = tPageEnd - tPageStart;
      pageRtts.push(pageRtt);

      // UI Update
      const progress = (pIdx + 1) / APP_TOTAL_PAGES;
      progressBar.style.width = `${(progress * 100).toFixed(1)}%`;
      progressPercent.textContent = `${(progress * 100).toFixed(1)}%`;
      progressSubText.textContent = `Page ${pageNo} / 255 (RTT: ${pageRtt.toFixed(1)}ms)`;

      const elapsedSec = (performance.now() - tStart) / 1000.0;
      const kbSec = ((pIdx + 1) * 64 / 1024.0) / elapsedSec;
      metricSpeed.textContent = `${kbSec.toFixed(2)} KB/s`;
      metricElapsed.textContent = `${elapsedSec.toFixed(2)}s`;
      metricAvgRtt.textContent = `${(pageRtts.reduce((a, b) => a + b, 0) / pageRtts.length).toFixed(1)}ms`;
      metricCurrentPage.textContent = `${pageNo}`;
    }

    const tTotalSec = (performance.now() - tStart) / 1000.0;
    log(`★ 12KB フル OTW 書き換え完了! 所要時間: ${tTotalSec.toFixed(2)} 秒 (実効速度: ${(12.0 / tTotalSec).toFixed(2)} KB/s)`, 'success');

    // Auto Launch
    log(`新ファームウェア自動起動コマンド (0x14) を発行中...`, 'info');
    const execFrame = buildMr32Frame(nodeId, 0x00, CMD_BOOT_APP_EXEC, 1);
    const execRes = await serialMgr.sendAndReceive(execFrame, 150);
    if (execRes.ok) {
      log(`★ アプリケーション自動起動成功! 基板の赤・白LED交互点滅を確認してください!`, 'success');
    }

  } catch (err) {
    log(`OTW書込中断: ${err.message}`, 'error');
  } finally {
    serialMgr.isTransferring = false;
    statusDot.className = 'status-dot connected';
    statusText.textContent = 'CONNECTED';
    btnFlashOTW.disabled = false;
    btnPing.disabled = false;
    btnProtectTest.disabled = false;
  }
});

// Clear log
btnClearLog.addEventListener('click', () => {
  consoleOutput.innerHTML = '';
});

// Tab switching
document.querySelectorAll('.tab-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
    document.querySelectorAll('.tab-pane').forEach(p => p.classList.remove('active'));
    btn.classList.add('active');
    document.getElementById(btn.dataset.pane).classList.add('active');
  });
});

// PWA Service Worker Registration
if ('serviceWorker' in navigator && window.location.protocol.startsWith('http')) {
  window.addEventListener('load', () => {
    navigator.serviceWorker.register('./sw.js').then(reg => {
      console.log('PWA Service Worker registered:', reg.scope);
    }).catch(err => {
      console.warn('PWA SW registration failed:', err);
    });
  });
}
