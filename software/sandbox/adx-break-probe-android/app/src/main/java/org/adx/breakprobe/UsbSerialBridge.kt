package org.adx.breakprobe

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.Build
import android.os.SystemClock
import android.util.Log
import android.webkit.JavascriptInterface
import android.webkit.WebView
import com.hoho.android.usbserial.driver.UsbSerialDriver
import com.hoho.android.usbserial.driver.UsbSerialPort
import com.hoho.android.usbserial.driver.UsbSerialProber
import com.hoho.android.usbserial.util.SerialInputOutputManager
import org.json.JSONArray
import org.json.JSONObject
import java.io.IOException

class UsbSerialBridge(
    private val context: Context,
    private val webView: WebView
) : SerialInputOutputManager.Listener {

    companion object {
        private const val TAG = "UsbSerialBridge"
        const val ACTION_USB_PERMISSION = "org.adx.breakprobe.USB_PERMISSION"
    }

    private val usbManager: UsbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager
    private var serialPort: UsbSerialPort? = null
    private var ioManager: SerialInputOutputManager? = null
    private var pendingBaudRate: Int = 19200

    private val usbReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            if (ACTION_USB_PERMISSION == action) {
                synchronized(this) {
                    val device: UsbDevice? = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                        intent.getParcelableExtra(UsbManager.EXTRA_DEVICE, UsbDevice::class.java)
                    } else {
                        @Suppress("DEPRECATION")
                        intent.getParcelableExtra(UsbManager.EXTRA_DEVICE)
                    }
                    val granted = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)
                    if (granted && device != null) {
                        Log.i(TAG, "USB Permission GRANTED for device: ${device.deviceName}")
                        openDeviceInternal(device, pendingBaudRate)
                    } else {
                        Log.w(TAG, "USB Permission DENIED")
                        notifyJsStatus("PERMISSION_DENIED", "USB Permission denied by user.")
                    }
                }
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED == action) {
                Log.i(TAG, "USB device detached")
                disconnect()
                notifyJsStatus("DETACHED", "USB device disconnected.")
            }
        }
    }

    init {
        val filter = IntentFilter(ACTION_USB_PERMISSION).apply {
            addAction(UsbManager.ACTION_USB_DEVICE_DETACHED)
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            context.registerReceiver(usbReceiver, filter, Context.RECEIVER_NOT_EXPORTED)
        } else {
            context.registerReceiver(usbReceiver, filter)
        }
    }

    fun onDestroy() {
        try {
            context.unregisterReceiver(usbReceiver)
        } catch (_: Exception) {}
        disconnect()
    }

    @JavascriptInterface
    fun isAndroid(): Boolean = true

    @JavascriptInterface
    fun listDevices(): String {
        val availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager)
        val array = JSONArray()
        for (driver in availableDrivers) {
            val device = driver.device
            val obj = JSONObject()
            obj.put("deviceId", device.deviceId)
            obj.put("deviceName", device.deviceName)
            obj.put("vendorId", device.vendorId)
            obj.put("productId", device.productId)
            obj.put("manufacturerName", device.manufacturerName ?: "Unknown")
            obj.put("productName", device.productName ?: "Serial Port")
            array.put(obj)
        }
        return array.toString()
    }

    @JavascriptInterface
    fun connect(baudRate: Int): String {
        val availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager)
        if (availableDrivers.isEmpty()) {
            return JSONObject().apply {
                put("status", "NO_DEVICE")
                put("message", "No USB serial hardware found.")
            }.toString()
        }

        val driver = availableDrivers[0]
        val device = driver.device

        if (!usbManager.hasPermission(device)) {
            pendingBaudRate = baudRate
            val flags = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                PendingIntent.FLAG_MUTABLE
            } else {
                0
            }
            val permissionIntent = PendingIntent.getBroadcast(
                context, 0, Intent(ACTION_USB_PERMISSION), flags
            )
            usbManager.requestPermission(device, permissionIntent)
            return JSONObject().apply {
                put("status", "REQUESTING_PERMISSION")
                put("message", "Requesting OS USB permission...")
            }.toString()
        }

        return openDeviceInternal(device, baudRate)
    }

    private fun openDeviceInternal(device: UsbDevice, baudRate: Int): String {
        val availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager)
        val driver = availableDrivers.find { it.device.deviceId == device.deviceId } ?: availableDrivers.firstOrNull()
        if (driver == null) {
            return JSONObject().apply {
                put("status", "DRIVER_ERROR")
                put("message", "No matching driver found.")
            }.toString()
        }

        val connection = usbManager.openDevice(driver.device) ?: return JSONObject().apply {
            put("status", "OPEN_FAILED")
            put("message", "Failed to open USB connection.")
        }.toString()

        val port = driver.ports[0]
        try {
            port.open(connection)
            port.setParameters(baudRate, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE)
            port.dtr = true
            port.rts = true

            serialPort = port
            ioManager = SerialInputOutputManager(port, this).apply {
                readBufferSize = 1024
                start()
            }

            Log.i(TAG, "Port opened successfully @ $baudRate bps")
            notifyJsStatus("CONNECTED", "Port opened @ $baudRate bps")
            return JSONObject().apply {
                put("status", "CONNECTED")
                put("baudRate", baudRate)
                put("vid", device.vendorId)
                put("pid", device.productId)
            }.toString()
        } catch (e: IOException) {
            Log.e(TAG, "Error opening port", e)
            return JSONObject().apply {
                put("status", "ERROR")
                put("message", e.message ?: "IOException")
            }.toString()
        }
    }

    @JavascriptInterface
    fun disconnect(): Boolean {
        try {
            ioManager?.stop()
            ioManager = null
            serialPort?.close()
            serialPort = null
            notifyJsStatus("DISCONNECTED", "Port closed.")
            return true
        } catch (e: Exception) {
            Log.e(TAG, "Error closing port", e)
            return false
        }
    }

    @JavascriptInterface
    fun isConnected(): Boolean {
        return serialPort?.isOpen == true
    }

    @JavascriptInterface
    fun setBreak(assertBreak: Boolean): Boolean {
        val port = serialPort ?: return false
        return try {
            port.setBreak(assertBreak)
            true
        } catch (e: Exception) {
            Log.e(TAG, "Error setting BREAK=$assertBreak", e)
            false
        }
    }

    @JavascriptInterface
    fun setBaudRate(baudRate: Int): Boolean {
        val port = serialPort ?: return false
        return try {
            port.setParameters(baudRate, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE)
            true
        } catch (e: Exception) {
            Log.e(TAG, "Error setting baudRate=$baudRate", e)
            false
        }
    }

    @JavascriptInterface
    fun writeHex(hexStr: String): Boolean {
        val port = serialPort ?: return false
        return try {
            val bytes = hexStringToByteArray(hexStr)
            port.write(bytes, 500)
            true
        } catch (e: Exception) {
            Log.e(TAG, "Error writing data", e)
            false
        }
    }

    /**
     * Native Half-Baud Atomic Engine
     * Switches baud rate to 9600, sends 0x00 (pure LIN BREAK pulse), waits for physical UART transmission,
     * restores 19200 baud, waits delimiter, and sends 33-Byte atomic packet (0x55 + 32B frame)
     * WITHOUT closing the port or destroying the background reader thread.
     */
    @JavascriptInterface
    fun executeNativeHalfBaudTransaction(
        packetHex: String,
        flushWaitMs: Long,
        delimMs: Long
    ): String {
        val port = serialPort ?: return JSONObject().apply {
            put("status", "ERROR")
            put("message", "Port not open")
        }.toString()

        return try {
            // 1. Drop to 9600 bps via USB control transfer (instant, no port close)
            port.setParameters(9600, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE)

            // 2. Send 0x00 byte (Auto-DE circuit activates, produces perfect 18~20 bit-time LOW on RS-485 bus)
            port.write(byteArrayOf(0x00), 100)

            // 3. Wait for 9600 bps UART byte transmission to complete physically (12ms for safe delivery)
            val flushMs = if (flushWaitMs > 0) flushWaitMs else 12L
            SystemClock.sleep(flushMs)

            // 4. Restore 19200 bps instantly via USB control transfer
            port.setParameters(19200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE)

            // 5. Delimiter time (idle HIGH on TX, typically 8ms for clock settling)
            val delim = if (delimMs >= 0) delimMs else 8L
            if (delim > 0) {
                SystemClock.sleep(delim)
            }

            // 6. Send atomic 33-Byte packet (0x55 Sync + 32B Master Frame)
            val packetBytes = hexStringToByteArray(packetHex)
            port.write(packetBytes, 200)

            JSONObject().apply {
                put("status", "SUCCESS")
            }.toString()
        } catch (e: Exception) {
            Log.e(TAG, "Native Half-Baud transaction error", e)
            JSONObject().apply {
                put("status", "ERROR")
                put("message", e.message ?: "Native Half-Baud error")
            }.toString()
        }
    }

    override fun onNewData(data: ByteArray) {
        if (data.isNotEmpty()) {
            val hex = byteArrayToHexString(data)
            webView.post {
                webView.evaluateJavascript("window.onAndroidSerialRx && window.onAndroidSerialRx('$hex');", null)
            }
        }
    }

    override fun onRunError(e: Exception) {
        Log.e(TAG, "Serial IO Manager error", e)
        webView.post {
            notifyJsStatus("IO_ERROR", e.message ?: "Serial error")
        }
    }

    private fun notifyJsStatus(status: String, message: String) {
        webView.post {
            webView.evaluateJavascript(
                "window.onAndroidSerialStatus && window.onAndroidSerialStatus('$status', '${message.replace("'", "\\'")}');",
                null
            )
        }
    }

    private fun hexStringToByteArray(s: String): ByteArray {
        val clean = s.replace(" ", "").replace("\n", "")
        val len = clean.length
        val data = ByteArray(len / 2)
        var i = 0
        while (i < len) {
            data[i / 2] = ((Character.digit(clean[i], 16) shl 4) + Character.digit(clean[i + 1], 16)).toByte()
            i += 2
        }
        return data
    }

    private fun byteArrayToHexString(bytes: ByteArray): String {
        val sb = java.lang.StringBuilder()
        for (b in bytes) {
            sb.append(String.format("%02X", b))
        }
        return sb.toString()
    }
}
