@echo off
cmd /c %1 shell dumpsys package com.jingdong.app.mall | findstr versionCode
