REM run in current directory
CD /D %~dp0
REM build the 4klang.obj
.\nasm.exe -fwin32 -o "4klang.obj" 4klang.asm
REM copy them to the source folder
xcopy 4klang.h ..
xcopy 4klang.obj ..