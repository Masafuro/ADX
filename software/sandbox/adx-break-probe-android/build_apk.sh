#!/bin/bash
set -e
export ANDROID_HOME=/home/ubuntu/android-sdk
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export PATH=$JAVA_HOME/bin:$ANDROID_HOME/cmdline-tools/latest/bin:$PATH

echo "=== Starting Gradle AssembleDebug ==="
echo "Java Version:"
java -version
echo "Android SDK: $ANDROID_HOME"

cd /home/ubuntu/AgentWorkspace/ADX/github/ADX/software/sandbox/adx-break-probe-android
./gradlew assembleDebug --stacktrace
echo "=== Build Complete ==="
