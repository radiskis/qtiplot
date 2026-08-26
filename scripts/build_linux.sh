#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_linux"

echo "=== Building QtiPlot on Linux ==="
echo "Project root: ${PROJECT_ROOT}"
echo "Build dir:    ${BUILD_DIR}"

mkdir -p "${BUILD_DIR}/sip_temp"

echo "--- Generating SIP bindings ---"
cd "${PROJECT_ROOT}"
sip-build --build-dir "${BUILD_DIR}/sip_temp" --no-compile

echo "--- Configuring with CMake ---"
cmake -B "${BUILD_DIR}" -S "${PROJECT_ROOT}" \
    -G "Ninja" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_PYTHON=ON

echo "--- Compiling ---"
cmake --build "${BUILD_DIR}" --config Release

echo "--- Running Automated Tests (Headless) ---"
QT_QPA_PLATFORM=offscreen "${BUILD_DIR}/qtiplot/qtiplot" -X "${PROJECT_ROOT}/tests/pytest_runner.py"

echo "--- Creating Release Package (TGZ) ---"
cd "${BUILD_DIR}"
cpack -G TGZ

echo "=== Build, Tests, and Packaging Completed Successfully! ==="
