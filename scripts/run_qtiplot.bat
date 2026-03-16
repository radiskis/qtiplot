set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
set "QT_PATH=C:\Qt\6.10.2\msvc2022_64"
set "PATH=%QT_PATH%\bin;%PROJECT_ROOT%\build\3rdparty\muparser;%PROJECT_ROOT%\build\3rdparty\qwtplot3d;%PATH%"
set "QT_PLUGIN_PATH=%QT_PATH%\plugins"
set QT_DEBUG_PLUGINS=1

echo Starting QtiPlot...
"%PROJECT_ROOT%\build\qtiplot\qtiplot.exe" %*
if %ERRORLEVEL% NEQ 0 echo Exited with error code %ERRORLEVEL%
pause
