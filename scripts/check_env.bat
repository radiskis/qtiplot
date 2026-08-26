@echo off
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
echo Checking for cl.exe...
where cl.exe
echo Checking for ninja.exe / cmake.exe...
where ninja.exe
where cmake.exe

