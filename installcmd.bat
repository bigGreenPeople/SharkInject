SET ABI_PATH=x86
rem SET ABI_PATH=arm64-v8a
rem SET ABI_PATH=armeabi-v7a
SET DEVICE_NAME=127.0.0.1:21513
rem SET DEVICE_NAME=d42e0b6
rem SET DEVICE_NAME=MYV0215A20003885
SET APP_PATH=%~dp0app\libs\%ABI_PATH%
echo %APP_PATH%

cd /d %APP_PATH%

for %%i in (*.*) do (
echo %%i
adb -s %DEVICE_NAME% shell rm -f /data/local/tmp/%%i
adb -s %DEVICE_NAME% push %%i /data/local/tmp/
adb -s %DEVICE_NAME% shell chmod 777 /data/local/tmp/%%i
)

rem adb -s %DEVICE_NAME% uninstall com.example.androidinject
rem adb -s %DEVICE_NAME% install %~dp0app\build\outputs\apk\debug\app-debug.apk
rem adb -s %DEVICE_NAME% shell am start -n com.example.androidinject/com.example.androidinject.MainActivity
rem adb -s %DEVICE_NAME% shell su
rem adb -s %DEVICE_NAME% shell /data/local/tmp/PtraceInject

cmd