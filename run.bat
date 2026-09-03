@echo off
set "ROOT=%~dp0"
if exist "%ROOT%build\qtiplot\qtiplot.exe" (
    if not exist "%ROOT%build\qtiplot\clang_rt.asan_dynamic-x86_64.dll" (
        where clang_rt.asan_dynamic-x86_64.dll >nul 2>nul
        if %errorlevel% equ 0 (
            for /f "delims=" %%I in ('where clang_rt.asan_dynamic-x86_64.dll') do (
                copy /y "%%I" "%ROOT%build\qtiplot\" >nul 2>nul
            )
        ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC" (
            for /f "delims=" %%I in ('dir /s /b "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\clang_rt.asan_dynamic-x86_64.dll" 2^>nul') do (
                copy /y "%%I" "%ROOT%build\qtiplot\" >nul 2>nul
            )
        )
    )
    start "" "%ROOT%build\qtiplot\qtiplot.exe" %*
) else (
    echo Error: build\qtiplot\qtiplot.exe not found. Please run scripts\build_cmake.bat first.
    pause
)
