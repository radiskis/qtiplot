@echo off
setlocal

set "DO_PACKAGE=0"
for %%a in (%*) do (
    if /i "%%a"=="--package" set "DO_PACKAGE=1"
    if /i "%%a"=="-p" set "DO_PACKAGE=1"
    if /i "%%a"=="--no-package" set "DO_PACKAGE=0"
)

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
        if exist "C:\Qt\6.11.2\msvc2022_64" set "QT_DIR=C:\Qt\6.11.2\msvc2022_64"
        if not defined QT_DIR if exist "C:\Qt\6.10.2\msvc2022_64" set "QT_DIR=C:\Qt\6.10.2\msvc2022_64"
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

echo Checking Python bindings with SIP...
set "SIP_BUILD=sip-build"
set "NEED_SIP=0"
if not exist "%BUILD_DIR%\sip_temp\qti\sipqticmodule.cpp" (
    set "NEED_SIP=1"
) else (
    python -c "import os, glob; sip_files = glob.glob(r'%PROJECT_ROOT%\qtiplot\src\scripting\*.sip'); out_file = r'%BUILD_DIR%\sip_temp\qti\sipqticmodule.cpp'; out_mtime = os.path.getmtime(out_file); exit(0 if any(os.path.getmtime(f) > out_mtime for f in sip_files) else 1)"
    if not errorlevel 1 set "NEED_SIP=1"
)

if "%NEED_SIP%"=="1" (
    echo Generating Python bindings with SIP...
    if exist "%BUILD_DIR%\sip_temp" rmdir /s /q "%BUILD_DIR%\sip_temp"
    mkdir "%BUILD_DIR%\sip_temp"
    pushd "%PROJECT_ROOT%"
    "%SIP_BUILD%" --build-dir "%BUILD_DIR%\sip_temp" --no-compile
    if %errorlevel% neq 0 (
        echo Error: SIP binding generation failed.
        popd
        exit /b %errorlevel%
    )
    popd
) else (
    echo Python bindings are up-to-date. Skipping SIP generation.
)

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
    where clang_rt.asan_dynamic-x86_64.dll >nul 2>nul
    if %errorlevel% equ 0 (
        for /f "delims=" %%I in ('where clang_rt.asan_dynamic-x86_64.dll') do (
            copy /y "%%I" "%BUILD_DIR%\qtiplot\" >nul 2>nul
        )
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

if "%DO_PACKAGE%"=="1" (
    echo Creating release package with CPack...
    cpack
    if %errorlevel% neq 0 (
        echo Warning: CPack packaging failed.
    ) else (
        echo Release package created successfully in %BUILD_DIR%!
    )
) else (
    echo Build completed and verified. Unpackaged build kept for fast testing. Pass --package or -p to create release ZIP.
)
