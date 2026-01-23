@echo off
REM ============================================================================
REM Build with CUDA Support (requires MSVC)
REM ============================================================================

echo ============================================
echo   Building Nebula Architect with CUDA
echo   (Requires Visual Studio 2019/2022)
echo ============================================

REM Check for Visual Studio
where cl.exe >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: MSVC compiler not found!
    echo Please run this script from "Developer Command Prompt for VS"
    echo Or install Visual Studio with C++ workload
    echo.
    pause
    exit /b 1
)

REM Check for CUDA
where nvcc.exe >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CUDA toolkit not found!
    echo Please install CUDA Toolkit from: https://developer.nvidia.com/cuda-downloads
    echo.
    pause
    exit /b 1
)

REM Check vcpkg
if not defined VCPKG_ROOT (
    if exist "C:\vcpkg" (
        set VCPKG_ROOT=C:\vcpkg
    ) else if exist "%USERPROFILE%\vcpkg" (
        set VCPKG_ROOT=%USERPROFILE%\vcpkg
    ) else (
        echo.
        echo ERROR: vcpkg not found!
        echo Please install vcpkg or set VCPKG_ROOT environment variable
        echo.
        pause
        exit /b 1
    )
)

echo Using vcpkg at: %VCPKG_ROOT%

REM Install dependencies for MSVC
echo.
echo Installing dependencies for MSVC...
%VCPKG_ROOT%\vcpkg install glm:x64-windows raylib:x64-windows glad:x64-windows glfw3:x64-windows

REM Create build directory
if not exist "build-cuda" mkdir build-cuda
cd build-cuda

REM Configure with CMake
echo.
echo Configuring CMake with CUDA support...
cmake -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows ^
    -DUSE_CUDA=ON ^
    ..

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build
echo.
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ============================================
echo   Build successful!
echo   Executable: build-cuda\Release\UNIVERSE.exe
echo ============================================
pause
