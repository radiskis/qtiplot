set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
if not defined QT_PATH (
    if defined QT_DIR (
        set "QT_PATH=%QT_DIR%"
    ) else if defined QTDIR (
        set "QT_PATH=%QTDIR%"
    ) else (
        if exist "C:\Qt\6.10.2\msvc2022_64" set "QT_PATH=C:\Qt\6.10.2\msvc2022_64"
        if not defined QT_PATH if exist "C:\Qt\6.8.2\msvc2022_64" set "QT_PATH=C:\Qt\6.8.2\msvc2022_64"
        if not defined QT_PATH if exist "C:\Qt\6.5.3\msvc2022_64" set "QT_PATH=C:\Qt\6.5.3\msvc2022_64"
    )
)
if defined QT_PATH (
    set "PATH=%QT_PATH%\bin;%PROJECT_ROOT%\build\3rdparty\muparser;%PROJECT_ROOT%\build\3rdparty\qwtplot3d;%PATH%"
    set "QT_PLUGIN_PATH=%QT_PATH%\plugins"
) else (
    set "PATH=%PROJECT_ROOT%\build\3rdparty\muparser;%PROJECT_ROOT%\build\3rdparty\qwtplot3d;%PATH%"
)
set QT_DEBUG_PLUGINS=1

echo Starting QtiPlot...
"%PROJECT_ROOT%\build\qtiplot\qtiplot.exe" %*
if %ERRORLEVEL% NEQ 0 echo Exited with error code %ERRORLEVEL%
pause
