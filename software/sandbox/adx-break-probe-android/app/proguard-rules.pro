# Keep JavascriptInterface annotations and methods
-keepclassmembers class * {
    @android.webkit.JavascriptInterface <methods>;
}

# Keep UsbSerialBridge
-keep class org.adx.breakprobe.UsbSerialBridge { *; }
