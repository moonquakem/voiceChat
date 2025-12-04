# ====================================================================
# LightVoice: High-Performance C++20 Voice Chat Server
# CMakeLists.txt
#
# Main CMake build script for the LightVoice project.
# Author: Gemini
# ====================================================================

cmake_minimum_required(VERSION 3.20)
project(LightVoice VERSION 1.0 LANGUAGES CXX)

# --- Compiler and Standard Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Global Compile Options ---
# Enable Position Independent Code, crucial for shared libraries and some executables
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# General warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# --- Build Type Specific Settings ---
# For Release builds, enable aggressive optimizations
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
endif()

# For Debug builds, enable debugging symbols and disable optimizations
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# --- Dependencies ---
find_package(PkgConfig REQUIRED)

# 1. Threads (required by C++ standard library)
find_package(Threads REQUIRED)

# 2. Protobuf (for signaling)
# Must be found before subdirectories to make protobuf_generate_cpp available.
find_package(Protobuf 3.20 REQUIRED)

# Conditionally find other dependencies based on the build environment
if(NOT MSVC)
    # On non-Windows systems, use pkg-config to find system libraries.
    # This is generally more reliable with package managers like apt.
    message(STATUS "Using pkg-config for dependency detection on non-MSVC platform.")

    pkg_check_modules(spdlog REQUIRED IMPORTED_TARGET spdlog)
    set(SPDLOG_TARGET PkgConfig::spdlog)

    pkg_check_modules(fmt REQUIRED IMPORTED_TARGET fmt)
    set(FMT_TARGET PkgConfig::fmt)

    pkg_check_modules(Opus REQUIRED IMPORTED_TARGET opus)
    set(OPUS_TARGET PkgConfig::Opus)

    pkg_check_modules(PortAudio REQUIRED IMPORTED_TARGET portaudio-2.0)
    set(PORTAUDIO_TARGET PkgConfig::PortAudio)
else()
    # On Windows, find_package works well with package managers like vcpkg.
    message(STATUS "Using find_package for dependency detection on MSVC platform.")

    find_package(spdlog 1.12 REQUIRED)
    set(SPDLOG_TARGET spdlog::spdlog)

    find_package(fmt 10 REQUIRED)
    set(FMT_TARGET fmt::fmt)

    find_package(Opus 1.3 REQUIRED)
    set(OPUS_TARGET Opus::opus)

    find_package(PortAudio REQUIRED)
    set(PORTAUDIO_TARGET PortAudio::portaudio)
endif()

# --- Project Structure ---
# Add common library path for includes
include_directories(src)

# --- Subdirectories ---
# Protobuf code generation
add_subdirectory(src/proto)
# Server source code
add_subdirectory(src)
# Benchmark executables
add_subdirectory(benchmark)

# --- Final Message ---
message(STATUS "--------------------------------------------------")
message(STATUS "LightVoice Project Configuration Summary:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "--- Dependencies Found ---")
message(STATUS "  Protobuf: ${Protobuf_VERSION}")
message(STATUS "  spdlog: ${spdlog_VERSION}")
message(STATUS "  fmt: ${fmt_VERSION}")
message(STATUS "  Opus: ${Opus_VERSION}")
message(STATUS "  PortAudio: ${PortAudio_VERSION}")
message(STATUS "--------------------------------------------------")
