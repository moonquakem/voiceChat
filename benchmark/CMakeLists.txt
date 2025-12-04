# ====================================================================
# LightVoice: Benchmark CMakeLists.txt
#
# This file compiles the benchmark executables.
# ====================================================================

# --- Source Files ---
set(STRESS_TEST_SRC stress_test.cpp)
set(MIXER_BENCHMARK_SRC mixer_benchmark.cpp)

# --- Create Executables ---
add_executable(stress_test ${STRESS_TEST_SRC})
add_executable(mixer_benchmark ${MIXER_BENCHMARK_SRC})

# --- Link Libraries ---
target_link_libraries(stress_test
    PRIVATE
    ${SPDLOG_TARGET}
    ${FMT_TARGET}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(mixer_benchmark
    PRIVATE
    lightvoice_proto
    ${Protobuf_LIBRARIES}
    ${SPDLOG_TARGET}
    ${FMT_TARGET}
    ${OPUS_TARGET}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Link against the server's object files for access to classes if needed.
# This is a simple way to avoid creating a separate library for server components.
target_link_libraries(mixer_benchmark PRIVATE lightvoice_server)


# --- Set Output Directory ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
