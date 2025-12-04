# ====================================================================
# LightVoice: Main Src CMakeLists.txt
#
# This file compiles the main server executable.
# ====================================================================

# --- Find Subdirectories ---
# (None in this case, but could add src/http, etc.)

# --- Source Files ---
# Automatically gather all .cc files in the current directory and subdirectories
file(GLOB_RECURSE SRC_FILES
    "net/*.cc"
    "codec/*.cc"
    "room/*.cc"
    "timer/*.cc"
    "pool/*.cc"
)

# Add the main entry point
list(APPEND SRC_FILES "main.cc")

# --- Create Executable ---
add_executable(lightvoice_server ${SRC_FILES})
add_executable(test_client src/client/test_client.cpp)


# --- Link Libraries ---
target_link_libraries(lightvoice_server
    PRIVATE
    lightvoice_proto      # Our generated protobuf library
    ${Protobuf_LIBRARIES} # Google's protobuf runtime
    ${CMAKE_THREAD_LIBS_INIT} # pthreads
    ${SPDLOG_TARGET}        # Logging library
    ${FMT_TARGET}              # Formatting library
    ${OPUS_TARGET}            # Opus codec
)

target_link_libraries(test_client
    PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${SPDLOG_TARGET}
    ${FMT_TARGET}
    ${OPUS_TARGET}
    ${PORTAUDIO_TARGET}
)

# Set properties for installation (optional)
install(TARGETS lightvoice_server test_client
    RUNTIME DESTINATION bin
)

# --- Add bin directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
