@echo off
setlocal EnableExtensions

cd /d "%~dp0"

set "APP_NAME=HelloCustomWidgets"
set "APP_SUB_NAME=showcase"
set "PORT_NAME=pc"
set "OUTPUT_DIR=%CD%\output"
set "MAKE_CMD=make"
set "BUILD_ONLY=0"
set "SKIP_BUILD=0"
set "MAKE_ARGS="

:parse_args
if "%~1"=="" goto after_parse
if /I "%~1"=="--help" goto print_help
if /I "%~1"=="-h" goto print_help
if /I "%~1"=="--build-only" (
    set "BUILD_ONLY=1"
    shift
    goto parse_args
)
if /I "%~1"=="--no-build" (
    set "SKIP_BUILD=1"
    shift
    goto parse_args
)
set "MAKE_ARGS=%MAKE_ARGS% %~1"
shift
goto parse_args

:after_parse
where "%MAKE_CMD%" >nul 2>nul
if errorlevel 1 (
    if exist "C:\msys64\mingw64\bin\make.exe" (
        set "MAKE_CMD=C:\msys64\mingw64\bin\make.exe"
    ) else (
        echo [start-app] make was not found in PATH.
        echo [start-app] Install MSYS2/MinGW make or run from a shell where make is available.
        goto fail
    )
)

if "%SKIP_BUILD%"=="1" goto run_app

echo [start-app] Building %APP_NAME%/%APP_SUB_NAME% for %PORT_NAME%...
"%MAKE_CMD%" all APP=%APP_NAME% APP_SUB=%APP_SUB_NAME% PORT=%PORT_NAME% COMPILE_DEBUG= COMPILE_OPT_LEVEL=-O0 %MAKE_ARGS%
if errorlevel 1 (
    echo [start-app] Build failed.
    goto fail
)

if "%BUILD_ONLY%"=="1" (
    echo [start-app] Build completed.
    exit /b 0
)

:run_app
if not exist "%OUTPUT_DIR%\main.exe" (
    echo [start-app] Executable not found: "%OUTPUT_DIR%\main.exe"
    echo [start-app] Run start_app.bat without --no-build first.
    goto fail
)

echo [start-app] Starting "%OUTPUT_DIR%\main.exe"...
pushd "%OUTPUT_DIR%"
".\main.exe"
set "APP_EXIT_CODE=%ERRORLEVEL%"
popd
exit /b %APP_EXIT_CODE%

:print_help
echo Usage: start_app.bat [--build-only] [--no-build] [make variables]
echo.
echo Default target:
echo   APP=%APP_NAME% APP_SUB=%APP_SUB_NAME% PORT=%PORT_NAME%
echo.
echo Examples:
echo   start_app.bat
echo   start_app.bat --build-only
echo   start_app.bat --no-build
echo   start_app.bat BITS=32
exit /b 0

:fail
echo.
pause
exit /b 1
