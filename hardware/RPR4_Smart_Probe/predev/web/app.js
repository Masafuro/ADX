// ============================================================================
// ADX RPR4 Smart Probe - Web MIDI Protocol Driver & Benchmark (M1)
// ============================================================================

const SYSEX_START = 0xF0;
const SYSEX_END = 0xF7;
const MANUFACTURER_EXP = 0x7D;

const CMD_PING = 0x01;
const CMD_ECHO = 0x02;

const RESP_FLAG = 0x40;
const RESP_PONG = CMD_PING | RESP_FLAG; // 0x41
const RESP_ECHO = CMD_ECHO | RESP_FLAG; // 0x42

const STATUS_OK = 0x00;

let midiAccess = null;
let currentInput = null;
let currentOutput = null;

let packetSeq = 0;
let pendingResolvers = new Map(); // seq -> { resolve, reject, startTime, rawSent }

// DOM 要素
const btnRequestMidi = document.getElementById('btn-request-midi');
const selectMidiIn = document.getElementById('midi-in-select');
const selectMidiOut = document.getElementById('midi-out-select');
const btnConnect = document.getElementById('btn-connect');
const connBadge = document.getElementById('conn-badge');

const btnPing = document.getElementById('btn-ping');
const pingResult = document.getElementById('ping-result');
const pingInfo = document.getElementById('ping-info');
const pingRtt = document.getElementById('ping-rtt');

const echoInput = document.getElementById('echo-input');
const btnEcho = document.getElementById('btn-echo');
const echoStatus = document.getElementById('echo-status');

const chunkSizeSelect = document.getElementById('chunk-size');
const btnBenchmark = document.getElementById('btn-benchmark');
const benchProgress = document.getElementById('bench-progress');
const statSpeed = document.getElementById('stat-speed');
const statTime = document.getElementById('stat-time');
const statLoss = document.getElementById('stat-loss');
const statRtt = document.getElementById('stat-rtt');
const benchEval = document.getElementById('bench-eval');

const logConsole = document.getElementById('log-console');
const btnClearLog = document.getElementById('btn-clear-log');

// ログ出力ユーティリティ
function log(msg, type = 'info') {
  const time = new Date().toISOString().substring(11, 23);
  const div = document.createElement('div');
  div.className = `log-${type}`;
  div.textContent = `[${time}] ${msg}`;
  logConsole.appendChild(div);
  logConsole.scrollTop = logConsole.scrollHeight;
}

btnClearLog.addEventListener('click', () => {
  logConsole.innerHTML = '';
});

// ----------------------------------------------------------------------------
// 7-to-8 パッキング & アンパッキング
// ----------------------------------------------------------------------------
function pack7to8(rawBytes) {
  const packed = [];
  const len = rawBytes.length;
  let inIdx = 0;

  while (inIdx < len) {
    const chunkLen = Math.min(7, len - inIdx);
    let msbByte = 0;

    for (let i = 0; i < chunkLen; i++) {
      if (rawBytes[inIdx + i] & 0x80) {
        msbByte |= (1 << (6 - i));
      }
    }

    packed.push(msbByte & 0x7F);

    for (let i = 0; i < chunkLen; i++) {
      packed.push(rawBytes[inIdx + i] & 0x7F);
    }

    inIdx += chunkLen;
  }

  return new Uint8Array(packed);
}

function unpack7to8(packedBytes, rawLen) {
  const raw = [];
  let inIdx = 0;

  while (inIdx < packedBytes.length && raw.length < rawLen) {
    const msbByte = packedBytes[inIdx++];
    const chunkLen = Math.min(7, rawLen - raw.length, packedBytes.length - inIdx);

    for (let i = 0; i < chunkLen; i++) {
      let b = packedBytes[inIdx++];
      if (msbByte & (1 << (6 - i))) {
        b |= 0x80;
      }
      raw.push(b);
    }
  }

  return new Uint8Array(raw);
}

function calcChecksum(data) {
  let csum = 0;
  for (let i = 0; i < data.length; i++) {
    csum ^= (data[i] & 0x7F);
  }
  return csum & 0x7F;
}

function toHexStr(bytes) {
  return Array.from(bytes).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
}

// ----------------------------------------------------------------------------
// SysEx メッセージ送受信
// ----------------------------------------------------------------------------
function sendSysEx(cmd, rawPayload = new Uint8Array(0)) {
  if (!currentOutput) {
    return Promise.reject(new Error("MIDI 出力ポートが接続されていません"));
  }

  const seq = (packetSeq++) & 0x7F;
  const packedPayload = pack7to8(rawPayload);
  const payloadLen = rawPayload.length;

  const header = [
    MANUFACTURER_EXP,
    cmd & 0x7F,
    seq,
    (payloadLen >> 7) & 0x7F,
    payloadLen & 0x7F
  ];

  const body = [...header, ...packedPayload];
  const csum = calcChecksum(body);
  const fullPacket = new Uint8Array([SYSEX_START, ...body, csum, SYSEX_END]);

  return new Promise((resolve, reject) => {
    const timeout = setTimeout(() => {
      if (pendingResolvers.has(seq)) {
        pendingResolvers.delete(seq);
        reject(new Error(`タイムアウト (Seq: 0x${seq.toString(16)})`));
      }
    }, 1500);

    pendingResolvers.set(seq, {
      resolve: (res) => { clearTimeout(timeout); resolve(res); },
      reject: (err) => { clearTimeout(timeout); reject(err); },
      startTime: performance.now(),
      rawSent: rawPayload
    });

    try {
      currentOutput.send(fullPacket);
    } catch (e) {
      clearTimeout(timeout);
      pendingResolvers.delete(seq);
      reject(e);
    }
  });
}

function handleMidiMessage(event) {
  const data = event.data;
  if (!data || data.length < 8) return;
  if (data[0] !== SYSEX_START || data[data.length - 1] !== SYSEX_END) return;
  if (data[1] !== MANUFACTURER_EXP) return;

  const respCmd = data[2];
  const seq = data[3];
  const status = data[4];
  const declaredLen = (data[5] << 7) | data[6];

  const expectedCsum = data[data.length - 2];
  const bodyForCsum = data.slice(1, data.length - 2);
  const calcCsum = calcChecksum(bodyForCsum);

  if (expectedCsum !== calcCsum) {
    log(`チェックサムエラー: 受信 0x${expectedCsum.toString(16)} != 計算 0x${calcCsum.toString(16)}`, 'err');
    return;
  }

  const packedPayload = data.slice(7, data.length - 2);
  const rawPayload = unpack7to8(packedPayload, declaredLen);

  if (pendingResolvers.has(seq)) {
    const { resolve, startTime } = pendingResolvers.get(seq);
    pendingResolvers.delete(seq);
    const rtt = performance.now() - startTime;
    resolve({ respCmd, seq, status, rawPayload, rtt });
  }
}

// ----------------------------------------------------------------------------
// Web MIDI API 初期化 & UI バインディング
// ----------------------------------------------------------------------------
btnRequestMidi.addEventListener('click', async () => {
  if (!navigator.requestMIDIAccess) {
    log("このブラウザは Web MIDI API をサポートしていません。(Chrome/Edge/Firefoxを推奨)", 'err');
    alert("このブラウザは Web MIDI API をサポートしていません。Chrome または Firefox を使用してください。");
    return;
  }

  try {
    log("Web MIDI API アクセス要求中 (SysEx: true)...", 'info');
    midiAccess = await navigator.requestMIDIAccess({ sysex: true });
    log("Web MIDI アクセス権限を取得しました。", 'info');

    populateMidiPorts();
    midiAccess.onstatechange = () => populateMidiPorts();
  } catch (err) {
    log(`Web MIDI アクセス拒否またはエラー: ${err.message}`, 'err');
  }
});

function populateMidiPorts() {
  selectMidiIn.innerHTML = '';
  selectMidiOut.innerHTML = '';

  let inCount = 0;
  let outCount = 0;
  let autoInId = null;
  let autoOutId = null;

  for (const entry of midiAccess.inputs.values()) {
    const opt = document.createElement('option');
    opt.value = entry.id;
    opt.textContent = `${entry.name || 'Unknown'} (${entry.manufacturer || 'Generic'})`;
    selectMidiIn.appendChild(opt);
    inCount++;

    if ((entry.name && (entry.name.includes('RPR4') || entry.name.includes('Pico') || entry.name.includes('TinyUSB'))) || !autoInId) {
      autoInId = entry.id;
    }
  }

  for (const entry of midiAccess.outputs.values()) {
    const opt = document.createElement('option');
    opt.value = entry.id;
    opt.textContent = `${entry.name || 'Unknown'} (${entry.manufacturer || 'Generic'})`;
    selectMidiOut.appendChild(opt);
    outCount++;

    if ((entry.name && (entry.name.includes('RPR4') || entry.name.includes('Pico') || entry.name.includes('TinyUSB'))) || !autoOutId) {
      autoOutId = entry.id;
    }
  }

  selectMidiIn.disabled = (inCount === 0);
  selectMidiOut.disabled = (outCount === 0);
  btnConnect.disabled = (inCount === 0 || outCount === 0);

  if (autoInId) selectMidiIn.value = autoInId;
  if (autoOutId) selectMidiOut.value = autoOutId;

  log(`利用可能な MIDI ポート: In=${inCount}, Out=${outCount}`, 'info');
}

btnConnect.addEventListener('click', () => {
  const inId = selectMidiIn.value;
  const outId = selectMidiOut.value;

  if (currentInput) {
    currentInput.onmidimessage = null;
  }

  currentInput = midiAccess.inputs.get(inId);
  currentOutput = midiAccess.outputs.get(outId);

  if (currentInput && currentOutput) {
    currentInput.onmidimessage = handleMidiMessage;

    connBadge.className = 'badge badge-connected';
    connBadge.textContent = '接続中';
    btnPing.disabled = false;
    btnEcho.disabled = false;
    btnBenchmark.disabled = false;

    log(`ポート接続完了: ${currentInput.name} <==> ${currentOutput.name}`, 'info');
  } else {
    log("選択されたポートのオープンに失敗しました。", 'err');
  }
});

// ----------------------------------------------------------------------------
// PING テスト
// ----------------------------------------------------------------------------
btnPing.addEventListener('click', async () => {
  try {
    btnPing.disabled = true;
    log("CMD_PING (0x01) 送信中...", 'tx');
    const res = await sendSysEx(CMD_PING);

    const decoder = new TextDecoder();
    const infoText = decoder.decode(res.rawPayload);

    pingResult.style.display = 'block';
    pingInfo.textContent = infoText;
    pingRtt.textContent = `${res.rtt.toFixed(1)} ms`;

    log(`[PONG 受信] RTT: ${res.rtt.toFixed(1)}ms | Info: ${infoText}`, 'rx');
  } catch (err) {
    log(`PING 失敗: ${err.message}`, 'err');
    alert(`PING 失敗: ${err.message}`);
  } finally {
    btnPing.disabled = false;
  }
});

// ----------------------------------------------------------------------------
// 単発エコーテスト
// ----------------------------------------------------------------------------
btnEcho.addEventListener('click', async () => {
  const text = echoInput.value;
  const encoder = new TextEncoder();
  const rawBytes = encoder.encode(text);

  try {
    btnEcho.disabled = true;
    log(`CMD_ECHO 送信 (${rawBytes.length}B): "${text}"`, 'tx');

    const res = await sendSysEx(CMD_ECHO, rawBytes);
    const decoder = new TextDecoder();
    const echoText = decoder.decode(res.rawPayload);

    const match = (text === echoText);
    if (match) {
      echoStatus.innerHTML = `<span style="color: var(--success); font-weight: bold;">[PASS] 完全一致 (${res.rtt.toFixed(1)}ms)</span>`;
      log(`[ECHO 受信] 一致確認 OK! (RTT: ${res.rtt.toFixed(1)}ms)`, 'rx');
    } else {
      echoStatus.innerHTML = `<span style="color: var(--danger); font-weight: bold;">[FAIL] データ不一致</span>`;
      log(`[ECHO 不一致] 送信: "${text}" != 受信: "${echoText}"`, 'err');
    }
  } catch (err) {
    echoStatus.innerHTML = `<span style="color: var(--danger);">エラー: ${err.message}</span>`;
    log(`エコー失敗: ${err.message}`, 'err');
  } finally {
    btnEcho.disabled = false;
  }
});

// ----------------------------------------------------------------------------
// 16KB スループットベンチマーク (M1 最重要評価)
// ----------------------------------------------------------------------------
btnBenchmark.addEventListener('click', async () => {
  const TOTAL_BYTES = 16 * 1024; // 16KB (ADX ATtiny1616 Flash全容量)
  const chunkSize = parseInt(chunkSizeSelect.value, 10);
  const totalChunks = Math.ceil(TOTAL_BYTES / chunkSize);

  btnBenchmark.disabled = true;
  btnPing.disabled = true;
  btnEcho.disabled = true;
  benchEval.textContent = '';
  benchProgress.style.width = '0%';

  log(`=== 16KB ベンチマーク開始 (チャンク: ${chunkSize}B, 計 ${totalChunks} パケット往復) ===`, 'info');

  let totalRtt = 0;
  let errors = 0;
  const startTime = performance.now();

  for (let i = 0; i < totalChunks; i++) {
    // ランダムテストバイナリ生成
    const testChunk = new Uint8Array(chunkSize);
    for (let j = 0; j < chunkSize; j++) {
      testChunk[j] = (i + j) & 0xFF;
    }

    try {
      const res = await sendSysEx(CMD_ECHO, testChunk);

      // バイナリ一致検証
      let match = (res.rawPayload.length === testChunk.length);
      if (match) {
        for (let k = 0; k < testChunk.length; k++) {
          if (res.rawPayload[k] !== testChunk[k]) {
            match = false;
            break;
          }
        }
      }

      if (!match) {
        errors++;
      } else {
        totalRtt += res.rtt;
      }
    } catch (err) {
      errors++;
      log(`Chunk #${i + 1} 送信エラー: ${err.message}`, 'err');
    }

    // 進捗更新
    const progress = Math.round(((i + 1) / totalChunks) * 100);
    benchProgress.style.width = `${progress}%`;
  }

  const totalTimeMs = performance.now() - startTime;
  const speedKbps = ((TOTAL_BYTES / 1024) / (totalTimeMs / 1000)).toFixed(2);
  const avgRtt = (totalRtt / (totalChunks - errors || 1)).toFixed(2);
  const lossRate = ((errors / totalChunks) * 100).toFixed(2);

  // 統計表示
  statSpeed.textContent = speedKbps;
  statTime.textContent = Math.round(totalTimeMs);
  statLoss.textContent = `${errors} (${lossRate}%)`;
  statRtt.textContent = avgRtt;

  // M1 受入基準の判定 (目標: 20KB/s以上 & ロス0%)
  if (errors === 0 && parseFloat(speedKbps) >= 20.0) {
    benchEval.style.color = 'var(--success)';
    benchEval.textContent = `[PASS] M1 受入基準クリア! (実効 ${speedKbps} KB/s, ロス 0%)`;
    log(`[ベンチマーク完了] PASS: ${speedKbps} KB/s, 時間: ${Math.round(totalTimeMs)}ms`, 'rx');
  } else if (errors === 0) {
    benchEval.style.color = 'var(--warning)';
    benchEval.textContent = `[WARN] ロス 0% ですが速度が目標未満 (${speedKbps} KB/s < 20 KB/s)`;
    log(`[ベンチマーク完了] WARN: ${speedKbps} KB/s (目標 20 KB/s)`, 'warn');
  } else {
    benchEval.style.color = 'var(--danger)';
    benchEval.textContent = `[FAIL] パケットエラーが発生しました (${errors} 件)`;
    log(`[ベンチマーク完了] FAIL: ${errors} 件のエラー`, 'err');
  }

  btnBenchmark.disabled = false;
  btnPing.disabled = false;
  btnEcho.disabled = false;
});

