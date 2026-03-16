@echo off
setlocal

set QT_DIR=D:/Software/Qt/6.10.1/mingw_64 
set MINGW_DIR=D:/Software/Qt/Tools/mingw1310_64/bin
set BUILD_DIR=build

echo [1/3] Configuring...
cmake -S . -B %BUILD_DIR% ^
    -G "MinGW Makefiles" ^
    -DCMAKE_C_COMPILER="%MINGW_DIR%/gcc.exe" ^
    -DCMAKE_CXX_COMPILER="%MINGW_DIR%/g++.exe" ^
    -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo Configure failed!
    pause
    exit /b 1
)

echo [2/3] Building...
cmake --build %BUILD_DIR% --config Release --parallel
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo [3/3] Done! Executable: %BUILD_DIR%\AnywhereTranslate.exe
pause