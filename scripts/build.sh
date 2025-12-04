#!/bin/bash
# ====================================================================
# LightVoice: Build Script
#
# This script configures and builds the project in Release mode.
# Author: Gemini
# ====================================================================

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
BUILD_DIR="build"
BUILD_TYPE="Release" # Can be "Debug" or "Release"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) # Get number of cores

# --- Script Start ---
echo "========================================="
echo "  Building LightVoice"
echo "========================================="
echo "Build Directory:  ${BUILD_DIR}"
echo "Build Type:       ${BUILD_TYPE}"
echo "Parallel Jobs:    ${JOBS}"
echo "-----------------------------------------"

# --- Create Build Directory ---
echo "[1/4] Creating build directory..."
mkdir -p "${BUILD_DIR}"

# --- Run CMake ---
echo "[2/4] Configuring project with CMake..."
# Go into the build directory to run cmake
cd "${BUILD_DIR}"
cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..
cd ..

# --- Compile Project ---
echo "[3/4] Compiling project with make..."
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -- -j"${JOBS}"

# --- Post-Build ---
echo "[4/4] Creating symlink for executables..."
# Create a convenient bin directory at the root
mkdir -p bin
(cd bin && ln -sf ../build/bin/lightvoice_server lightvoice_server)
(cd bin && ln -sf ../build/bin/test_client test_client)
(cd bin && ln -sf ../build/bin/stress_test stress_test)
(cd bin && ln -sf ../build/bin/mixer_benchmark mixer_benchmark)


echo "========================================="
echo "  Build complete!"
echo "========================================="
echo "Executables are in '${BUILD_DIR}/bin' or linked in 'bin/'"
echo "To run the server: ./bin/lightvoice_server"
echo "To run the client: ./bin/test_client"
echo "========================================="
