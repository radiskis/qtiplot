@echo off
setlocal

echo Setting up environment...

REM Setup MSVC Environment (if cl is not already available)
where cl.exe >nul 2>nul
if %errorlevel% neq 0 (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            if exist "%%i\Common7\Tools\VsDevCmd.bat" (
                call "%%i\Common7\Tools\VsDevCmd.bat" -arch=x64
            )
        )
    )
)
where cl.exe >nul 2>nul
if %errorlevel% neq 0 (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
    )
)

REM Setup Qt Environment (support environment variable QT_DIR / QTDIR or detect standard paths)
if not defined QT_DIR (
    if defined QTDIR (
        set "QT_DIR=%QTDIR%"
    ) else (
        if exist "C:\Qt\6.10.2\msvc2022_64" set "QT_DIR=C:\Qt\6.10.2\msvc2022_64"
        if not defined QT_DIR if exist "C:\Qt\6.8.2\msvc2022_64" set "QT_DIR=C:\Qt\6.8.2\msvc2022_64"
        if not defined QT_DIR if exist "C:\Qt\6.5.3\msvc2022_64" set "QT_DIR=C:\Qt\6.5.3\msvc2022_64"
    )
)
if defined QT_DIR (
    set "PATH=%QT_DIR%\bin;%PATH%"
)

REM Setup CMake and Ninja Environment (if not already in PATH)
where cmake.exe >nul 2>nul
if %errorlevel% neq 0 (
    if exist "C:\Qt\Tools\CMake_64\bin" set "PATH=C:\Qt\Tools\CMake_64\bin;%PATH%"
    if exist "C:\Program Files\CMake\bin" set "PATH=C:\Program Files\CMake\bin;%PATH%"
)
where ninja.exe >nul 2>nul
if %errorlevel% neq 0 (
    if exist "C:\Qt\Tools\Ninja" set "PATH=C:\Qt\Tools\Ninja;%PATH%"
)

REM Setup Python and SIP Environment
for /f "delims=" %%i in ('python -c "import sysconfig; print(sysconfig.get_path('scripts', 'nt_user'))" 2^>nul') do (
    if exist "%%i" set "PATH=%%i;%PATH%"
)
for /f "delims=" %%i in ('python -c "import sysconfig; print(sysconfig.get_path('scripts'))" 2^>nul') do (
    if exist "%%i" set "PATH=%%i;%PATH%"
)

echo Environment setup complete.
if defined QT_DIR echo QT_DIR=%QT_DIR%

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

echo Running automated tests...
"%EXE_PATH%" -X "%PROJECT_ROOT%\tests\pytest_runner.py"
if %errorlevel% neq 0 (
    echo Error: Automated tests failed.
    exit /b %errorlevel%
)
echo Automated tests passed successfully!

echo Creating release package with CPack...
cpack
if %errorlevel% neq 0 (
    echo Warning: CPack packaging failed.
) else (
    echo Release package created successfully in %BUILD_DIR%!
)
