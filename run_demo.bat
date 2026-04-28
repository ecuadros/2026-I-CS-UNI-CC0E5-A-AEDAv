@echo off
setlocal

rem 
set "PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%"

mingw32-make clean
if errorlevel 1 exit /b %errorlevel%

mingw32-make
if errorlevel 1 exit /b %errorlevel%

main.exe
endlocal
