@echo off

set WINSDK_HOME=D:\DevEnv\WinSDK_Win2003R2
set PATH=%WINSDK_HOME%\Bin;%MSVC1998_HOME%\Bin
set INCLUDE=%WINSDK_HOME%\Include;%MSVC1998_HOME%\Include
set LIB=%WINSDK_HOME%\Lib;%MSVC1998_HOME%\Lib

set BUILD_TYPE_NAME=msvcrt_x86_release
set SRC_PATH=%~dp0src
set INT_PATH=%~dp0int\%BUILD_TYPE_NAME%
set OUT_PATH=%~dp0out_%BUILD_TYPE_NAME%

if exist "%INT_PATH%" rmdir /s /q "%INT_PATH%"
mkdir "%INT_PATH%" || goto gg
if exist "%OUT_PATH%" rmdir /s /q "%OUT_PATH%"
mkdir "%OUT_PATH%" || goto gg

echo.Building resource file...
rc -fo %INT_PATH%\noborder.app.res %SRC_PATH%\noborder.rc || goto gg

call :compile boon_layeredwindow.c || goto gg
call :compile compat_dwmapi.c || goto gg
call :compile Core.cpp || goto gg
call :compile CursorOverlay.cpp || goto gg
call :compile DwmWindow.cpp || goto gg
call :compile main.cpp || goto gg
call :compile NotifyIcon.cpp || goto gg

echo Linking...
set LIB=%INT_PATH%;%LIB%
link @%~dp0\make_msvcrt.link ^
	-out:%OUT_PATH%\noborder_x86.exe ^
	-opt:nowin98 ^
	-machine:I386 || goto gg
echo.Build OK! See: %OUT_PATH%
echo.
pause
exit/b

:gg
echo.Build ERROR!
echo.
pause
exit/b

:compile
set file1=%SRC_PATH%\%~nx1
set file2=%INT_PATH%\noborder.app.%~n1.obj
set /p a=Compile: <nul
cl -nologo @make_msvcrt.cl %file1% -Fo%file2%
exit/b
