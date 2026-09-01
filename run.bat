@echo off
set "ROOT=%~dp0"
if exist "%ROOT%build\qtiplot\qtiplot.exe" (
    start "" "%ROOT%build\qtiplot\qtiplot.exe" %*
) else (
    echo Error: build\qtiplot\qtiplot.exe not found. Please run scripts\build_cmake.bat first.
    pause
)
