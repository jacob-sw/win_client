@echo off
cmd /c %1 shell mkdir -p %2
cmd /c %1 push %3 %2

adb shell 'find /sdcard/ -name "gps*.trace" -print0' | xargs -0 -n 1 adb pull