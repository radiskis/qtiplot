# Building QtiPlot

This document provides instructions for compiling, testing, and packaging QtiPlot from source on **Windows** and **Linux** using **CMake** and **Qt 6**.

---

## 1. Prerequisites & System Requirements

### Windows (MSVC 64-bit)

1. **C++ Compiler**:
   - Microsoft Visual Studio 2022 (Community, Professional, Enterprise, or Build Tools) with "Desktop development with C++" workload.
2. **Qt 6**:
   - Qt 6.5+ (tested on Qt 6.8 / 6.10, `msvc2022_64`).
   - Required Qt modules: `Widgets`, `Svg`, `Xml`, `OpenGL`, `OpenGLWidgets`, `PrintSupport`, `Concurrent`, `Network`.
   - Tool `windeployqt` for runtime deployment.
3. **Build Tools**:
   - **CMake** (version >= 3.16)
   - **Ninja** build system
4. **Python & Scripting Engine**:
   - Python 3.10+ (tested on Python 3.13 64-bit)
   - Required Python packages:
     ```cmd
     pip install sip PyQt6 PyQt6-sip numpy scipy sympy pytest
     ```

### Linux (Debian / Ubuntu / WSL2)

1. **C++ Compiler & Build Tools**:
   - GCC / G++ (version >= 10, C++17 support)
   - `cmake` (version >= 3.16), `ninja-build`, `git`
2. **Qt 6 Development Packages**:
   - Debian / Ubuntu / WSL:
     ```bash
     sudo apt update
     sudo apt install build-essential cmake ninja-build \
         qt6-base-dev qt6-base-dev-tools libqt6svg6-dev \
         libqt6opengl6-dev libgl1-mesa-dev libglu1-mesa-dev
     ```
3. **Python & Scripting Engine**:
   - Debian / Ubuntu / WSL:
     ```bash
     sudo apt install python3-dev python3-pip sip-tools \
         python3-pyqt6 python3-pyqt6.sip python3-scipy \
         python3-sympy python3-pytest python3-numpy
     ```

---

## 2. Building on Windows

### Quick One-Command Build

Run the automated build script:
```cmd
scripts\build_cmake.bat
```
This script will:
1. Auto-detect MSVC environment and Qt 6 installation.
2. Generate SIP 6 Python bindings dynamically.
3. Configure CMake with Ninja in Release mode.
4. Compile all bundled 3rdparty libraries and QtiPlot.
5. Deploy Qt runtime DLLs using `windeployqt`.
6. Run the 18-test automated pytest verification suite.
7. Package the standalone portable archive: `build\qtiplot-0.9.9-win64.zip`.

### Manual Build Steps

```cmd
:: 1. Open x64 Native Tools Command Prompt for VS 2022

:: 2. Set Qt directory (if not in PATH)
set "PATH=C:\Qt\6.11.2\msvc2022_64\bin;%PATH%"

:: 3. Generate SIP Python bindings
mkdir build\sip_temp
sip-build --build-dir build\sip_temp --no-compile

:: 4. Configure CMake
cmake -B build -S . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DENABLE_PYTHON=ON

:: 5. Build
cmake --build build --config Release

:: 6. Deploy Qt DLLs for standalone execution
windeployqt --release --no-translations --no-opengl-sw build\qtiplot\qtiplot.exe

:: 7. Create ZIP release package
cd build
cpack -G ZIP
```

---

## 3. Building on Linux / WSL

### Quick One-Command Build

Run the build script:
```bash
./scripts/build_linux.sh
```

### Manual Build Steps

```bash
# 1. Generate SIP bindings
mkdir -p build/sip_temp
sip-build --build-dir build/sip_temp --no-compile

# 2. Configure with CMake
cmake -B build -S . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DENABLE_PYTHON=ON

# 3. Compile
cmake --build build --config Release

# 4. Create TGZ release package
cd build
cpack -G TGZ
```

---

## 4. Running Automated Tests

QtiPlot includes an automated verification test suite powered by `pytest`.

To run tests in headless mode:

### Windows:
```cmd
build\qtiplot\qtiplot.exe -X tests\pytest_runner.py
```

### Linux (WSL / Offscreen):
```bash
QT_QPA_PLATFORM=offscreen build/qtiplot/qtiplot -X tests/pytest_runner.py
```

---

## 5. Running the Application

- **Windows**: Run `scripts\run_qtiplot.bat` or double-click `build\qtiplot\qtiplot.exe`.
- **Linux**: Run `build/qtiplot/qtiplot`.
