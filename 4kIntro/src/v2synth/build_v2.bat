REM run in current directory
CD /D %~dp0
REM build the tune.obj
.\nasmw.exe -fwin32 -o "tune.obj" tune.asm
REM copy them to the source folder
xcopy tune.obj .. /y
