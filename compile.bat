@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo  VectorDB Compiler Script (g++ / C++17)
echo ===================================================
echo.

set CXX=g++
if exist "..\w64devkit\bin\g++.exe" set CXX=..\w64devkit\bin\g++.exe
if exist "%USERPROFILE%\.gemini\antigravity\scratch\w64devkit\bin\g++.exe" set CXX=%USERPROFILE%\.gemini\antigravity\scratch\w64devkit\bin\g++.exe
if exist "C:\msys64\ucrt64\bin\g++.exe" set CXX=C:\msys64\ucrt64\bin\g++.exe
if exist "C:\msys64\mingw64\bin\g++.exe" set CXX=C:\msys64\mingw64\bin\g++.exe

echo Using compiler: "%CXX%"
echo.

echo [1/2] Compiling VectorDB Server (main.cpp)...
"%CXX%" -std=c++17 -O2 main.cpp -o db.exe -lws2_32 -D_WIN32_WINNT=0x0A00
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Failed to compile VectorDB Server!
    exit /b %errorlevel%
)
echo Server compiled successfully as db.exe.
echo.

echo [2/2] Compiling Unit Tests (src/tests.cpp)...
"%CXX%" -std=c++17 -O2 src/tests.cpp -o run_tests.exe -lws2_32 -D_WIN32_WINNT=0x0A00
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Failed to compile Unit Tests!
    exit /b %errorlevel%
)
echo Unit Tests compiled successfully as run_tests.exe.
echo.

echo ===================================================
echo  Compilation completed successfully!
echo  - Run the database server: .\db.exe
echo  - Run the test suite:      .\run_tests.exe
echo ===================================================
