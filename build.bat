@echo off
:: check if env variable for setup is set, if not call setup.bat and set environment variables
if not defined setup_env (
    call setup.bat
    if errorlevel 1 (
        echo Setup failed
        exit /b 1
    )
    echo Setup successful
    set setup_env=1
)
if not exist build mkdir build
if not exist third_party\OpenCL-Headers (
    mkdir third_party
    git clone https://github.com/KhronosGroup/OpenCL-Headers.git third_party\OpenCL-Headers
)

:: build then run the executable if successful
cl /nologo /TC /std:c17 /W3 ^
  /I"third_party\OpenCL-Headers" ^
  /Fo"build\main.obj" /Fe"build\main.exe" main.c ^
  /link /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.2\lib\x64" OpenCL.lib

if errorlevel 1 (
    echo Build failed
    exit /b 1
)
echo Build successful
build\main.exe
if errorlevel 1 (
    echo Run failed
    exit /b 1
)
