@echo off
%1 shell dumpsys package com.sea.cable.well | findstr versionCode
