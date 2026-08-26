# QtiPlot

QtiPlot is a powerful, user-friendly, cross-platform data analysis and scientific plotting application built with **Qt 6**, **C++17**, and integrated **Python 3 scripting**.

---

## Key Features

- **2D and 3D Plotting**: Curves, bars, pie charts, vector plots, histograms, spectrograms, surfaces, and custom multi-layer graph compositions.
- **Data Analysis & Fitting**: Linear, polynomial, exponential, Gauss, Lorentz, and user-defined non-linear curve fitting powered by GSL and ALGLIB.
- **Python Scripting Engine**: Full automation and batch processing capabilities with Python 3, PyQt6, SciPy, NumPy, and SymPy.
- **Cross-Platform**: Native support on Windows (MSVC 64-bit) and Linux (WSL2, Debian, Ubuntu, Fedora, Arch).
- **Import/Export**: Origin OPJ projects, Excel, ASCII, EMF, EPS, SVG, PNG, and PDF formats.

---

## Building from Source

Comprehensive build instructions and system prerequisites for **Windows** and **Linux** are detailed in [BUILD.md](BUILD.md).

### Quick Start

#### Windows (MSVC 64-bit):
```cmd
scripts\build_cmake.bat
```

#### Linux / WSL2:
```bash
./scripts/build_linux.sh
```

---

## Automated Tests

Run the automated pytest test suite:
```bash
# Windows
build\qtiplot\qtiplot.exe -X tests\pytest_runner.py

# Linux
QT_QPA_PLATFORM=offscreen build/qtiplot/qtiplot -X tests/pytest_runner.py
```

---

## License

QtiPlot is distributed under the [GNU General Public License v2 (GPL-2.0)](LICENSE).
