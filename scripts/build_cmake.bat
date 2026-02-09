@echo off
setlocal

echo Setting up environment...

REM Setup MSVC Environment
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
) else (
    echo Error: VsDevCmd.bat not found.
    exit /b 1
)
if %errorlevel% neq 0 (
    echo Error: Failed to setup MSVC environment.
    exit /b %errorlevel%
)

REM Setup Qt Environment
REM Qt location is still static based on tools.txt, as it is a specific version
set "QT_DIR=C:\Qt\6.10.2\msvc2022_64"
set "PATH=%QT_DIR%\bin;%PATH%"

REM Setup CMake and Ninja Environment
set "CMAKE_BIN=C:\Qt\Tools\CMake_64\bin"
set "NINJA_BIN=C:\Qt\Tools\Ninja"
set "PATH=%CMAKE_BIN%;%NINJA_BIN%;%PATH%"

REM Setup Python and SIP Environment
for /f "delims=" %%i in ('python -c "import sysconfig; print(sysconfig.get_path('scripts', 'nt_user'))"') do set "PYTHON_SCRIPTS=%%i"
set "PATH=%PYTHON_SCRIPTS%;%PATH%"

echo Environment setup complete.
echo QT_DIR=%QT_DIR%
echo CMAKE_BIN=%CMAKE_BIN%
echo NINJA_BIN=%NINJA_BIN%

REM Resolve paths
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
set "BUILD_DIR=%PROJECT_ROOT%\build"

echo Project Root: %PROJECT_ROOT%
echo Build Dir: %BUILD_DIR%

REM Create build directory
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

echo Generating Python bindings with SIP...
if exist "%BUILD_DIR%\sip_temp" (
    rmdir /s /q "%BUILD_DIR%\sip_temp"
)
mkdir "%BUILD_DIR%\sip_temp"
set "SIP_BUILD=sip-build"
pushd "%PROJECT_ROOT%"
"%SIP_BUILD%" --build-dir "%BUILD_DIR%\sip_temp" --no-compile
if %errorlevel% neq 0 (
    echo Error: SIP binding generation failed.
    popd
    exit /b %errorlevel%
)
popd

cd "%BUILD_DIR%"

echo Configuring with CMake (Ninja)...
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DENABLE_PYTHON=ON -DCMAKE_PREFIX_PATH="%QT_DIR%" "%PROJECT_ROOT%"
if %errorlevel% neq 0 (
    echo Error: CMake configuration failed.
    exit /b %errorlevel%
)

echo Building...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Error: Build failed.
    exit /b %errorlevel%
)

echo Build successful!

echo Deploying Qt runtime dependencies...
set "EXE_PATH=%BUILD_DIR%\qtiplot\qtiplot.exe"
if exist "%EXE_PATH%" (
    windeployqt --release --no-translations --no-opengl-sw --no-compiler-runtime --force "%EXE_PATH%"
    if %errorlevel% neq 0 (
        echo Warning: windeployqt failed.
    ) else (
        echo Deployment successful!
    )
) else (
    echo Error: qtiplot.exe not found at %EXE_PATH%
    exit /b 1
)
