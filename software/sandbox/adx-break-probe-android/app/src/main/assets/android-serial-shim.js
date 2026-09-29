/**
 * ADX Android Serial Shim (WebSerial 互換ブリッジ)
 * Android WebView 内で window.AndroidBridge が存在する場合、
 * ネイティブ USB-Serial 通信を WebSerial API 風に透過的にマッピングする。
 * かつ Native Half-Baud Atomic Engine を提供する。
 */
(function() {
  if (typeof window.AndroidBridge === 'undefined') {
    console.log('[AndroidSerialShim] Not running in Android WebView. Skipping shim.');
    return;
  }

  console.log('[AndroidSerialShim] Initializing Android USB-Serial bridge...');

  // 受信キュー & コールバックリスナー
  let rxQueue = [];
  let rxResolvers = [];
  let statusListeners = [];

  window.onAndroidSerialRx = function(hexString) {
    if (!hexString || hexString.length === 0) return;
    const len = hexString.length / 2;
    const bytes = new Uint8Array(len);
    for (let i = 0; i < len; i++) {
      bytes[i] = parseInt(hexString.substr(i * 2, 2), 16);
    }

    if (rxResolvers.length > 0) {
      const resolver = rxResolvers.shift();
      resolver({ value: bytes, done: false });
    } else {
      rxQueue.push(bytes);
    }
  };

  window.onAndroidSerialStatus = function(status, message) {
    console.log(`[AndroidSerialStatus] ${status}: ${message}`);
    statusListeners.forEach(fn => fn(status, message));
  };

  class AndroidSerialPort {
    constructor() {
      this.readable = {
        getReader: () => {
          return {
            read: async () => {
              if (rxQueue.length > 0) {
                const chunk = rxQueue.shift();
                return { value: chunk, done: false };
              }
              return new Promise((resolve) => {
                rxResolvers.push(resolve);
              });
            },
            releaseLock: () => {
              rxResolvers = [];
            },
            cancel: async () => {
              rxResolvers.forEach(res => res({ value: undefined, done: true }));
              rxResolvers = [];
              rxQueue = [];
            }
          };
        }
      };

      this.writable = {
        getWriter: () => {
          return {
            write: async (chunk) => {
              let hex = '';
              for (let i = 0; i < chunk.length; i++) {
                hex += chunk[i].toString(16).padStart(2, '0');
              }
              const ok = window.AndroidBridge.writeHex(hex);
              if (!ok) throw new Error('Native write failed');
            },
            releaseLock: () => {}
          };
        }
      };
    }

    async open(options = {}) {
      const baudRate = options.baudRate || 19200;
      const resJson = window.AndroidBridge.connect(baudRate);
      const res = JSON.parse(resJson);
      if (res.status === 'CONNECTED') {
        return;
      } else if (res.status === 'REQUESTING_PERMISSION') {
        // OSパーミッションダイアログの応答を待つ
        return new Promise((resolve, reject) => {
          const timeoutId = setTimeout(() => {
            reject(new Error('Permission request timed out.'));
          }, 30000);

          const listener = (status, msg) => {
            if (status === 'CONNECTED') {
              clearTimeout(timeoutId);
              statusListeners = statusListeners.filter(l => l !== listener);
              resolve();
            } else if (status === 'PERMISSION_DENIED' || status === 'ERROR') {
              clearTimeout(timeoutId);
              statusListeners = statusListeners.filter(l => l !== listener);
              reject(new Error(msg));
            }
          };
          statusListeners.push(listener);
        });
      } else {
        throw new Error(res.message || 'Failed to open serial port');
      }
    }

    async close() {
      window.AndroidBridge.disconnect();
      rxQueue = [];
      rxResolvers = [];
    }

    async setSignals(signals = {}) {
      if (typeof signals.break !== 'undefined') {
        window.AndroidBridge.setBreak(signals.break);
      }
    }
  }

  // navigator.serial を上書きまたはポリフィル提供
  const activePort = new AndroidSerialPort();

  window.onAndroidSerialClearRx = function() {
    rxQueue = [];
    rxResolvers = [];
  };

  window.AndroidSerialShim = {
    isAndroid: true,
    port: activePort,
    onStatus: (fn) => statusListeners.push(fn),
    clearRx: () => {
      rxQueue = [];
      rxResolvers = [];
    },
    /**
     * Native Half-Baud Atomic Transaction
     * Sends pure 18~20 bit LOW break via 9600 0x00 and sends 33B packet at 19200 without port close
     */
    executeNativeHalfBaud: async (packetBytes, flushWaitMs = 12, delimMs = 8) => {
      // 送信前に受信キューを完全フラッシュ（過去のゴミデータやエコーバックを破棄）
      rxQueue = [];
      rxResolvers = [];

      let hex = '';
      for (let i = 0; i < packetBytes.length; i++) {
        hex += packetBytes[i].toString(16).padStart(2, '0');
      }
      const resJson = window.AndroidBridge.executeNativeHalfBaudTransaction(hex, flushWaitMs, delimMs);
      const res = JSON.parse(resJson);
      if (res.status !== 'SUCCESS') {
        throw new Error(res.message || 'Native Half-Baud failed');
      }
      return true;
    }
  };

  // navigator.serial を Android ブリッジでラップ
  window.navigator.serial = {
    requestPort: async () => {
      return activePort;
    },
    getPorts: async () => {
      return [activePort];
    }
  };

  console.log('[AndroidSerialShim] Successfully polyfilled navigator.serial with Native Half-Baud Atomic support.');
})();
