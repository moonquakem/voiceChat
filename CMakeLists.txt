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
# 1. Threads (required by C++ standard library)
find_package(Threads REQUIRED)

# 2. Protobuf (for signaling)
find_package(Protobuf 3.20 REQUIRED)

# 3. spdlog and fmt (for logging)
find_package(spdlog 1.12 REQUIRED) # spdlog often bundles fmt
find_package(fmt 10 REQUIRED)

# 4. Opus (for audio codec)
find_package(Opus 1.3 REQUIRED)

# 5. PortAudio (for client audio I/O)
find_package(PortAudio REQUIRED)

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
