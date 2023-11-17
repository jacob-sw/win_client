@echo off
setlocal enabledelayedexpansion
for /F "tokens=* USEBACKQ" %%F in (`%1 shell ls %2`) do (
	set text=%%F
	set mfile=!text!
	%1 pull "!mfile!" %3
)
:end
endlocal