@echo off
title Dev Fanle Studio Compile

:: 提示用户选择操作
echo =====================================
echo  Dev Fanle Studio Compile
echo =====================================
echo.
echo 1. Normal compilation
echo 2. Compilation without console
echo 3. Run
echo 3. Exit
echo.
:Loop
set /p make=Operate:

:: 根据用户选择执行操作
if "%make%" == "1" (
    taskkill /f /t /im EditingInterface.exe
	del EditingInterface.exe
    g++ EditingInterface.cpp -o EditingInterface.exe -lcomctl32 -lcomdlg32 -lgdi32 -luser32 -static-libgcc -static-libstdc++
    start EditingInterface.exe
) else if "%make%" == "2" (
    taskkill /f /t /im EditingInterface.exe
	del EditingInterface.exe
    g++ -mwindows EditingInterface.cpp -o EditingInterface.exe -lcomctl32 -lcomdlg32 -lgdi32 -luser32 -static-libgcc -static-libstdc++
    start EditingInterface.exe
) else if "%make%" == "3" (
    start EditingInterface.exe
) else if "%make%" == "4" (
    exit
) else (
    echo There's nothing you want to do
    exit
)
goto :Loop