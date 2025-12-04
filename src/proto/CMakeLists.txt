# ====================================================================
# LightVoice: Protobuf Build Script
# src/proto/CMakeLists.txt
#
# Locates Protobuf and sets up rules to generate C++ sources from
# .proto files.
# Author: Gemini
# ====================================================================

# Find the Protobuf package, which is already done in the parent scope.
# This ensures the variables are available here.
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf is required but was not found.")
endif()

# --- Configuration for Proto Files ---
# Define the directory where .proto files are located.
set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Define the directory where generated files will be placed.
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# Include the generated directory for compiler to find the headers.
include_directories(${PROTO_GEN_DIR})

# --- Find All .proto Files ---
# Automatically find all .proto files in the source directory.
file(GLOB PROTO_FILES "${PROTO_SRC_DIR}/*.proto")

# --- Generate C++ Sources ---
# This function call creates a custom command to run protoc (the protobuf compiler)
# on our .proto files. It generates both the .pb.h and .pb.cc files.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# --- Create a Library ---
# Create a static library from the generated C++ source files.
# This library can then be linked against our main server executable.
add_library(lightvoice_proto STATIC ${PROTO_SRCS})

# --- Link Dependencies ---
# The generated code depends on the main Protobuf library (libprotobuf).
target_link_libraries(lightvoice_proto PRIVATE ${Protobuf_LIBRARIES})

# --- Message to User ---
message(STATUS "Protobuf C++ files will be generated from:")
foreach(FIL ${PROTO_FILES})
    message(STATUS "  - ${FIL}")
endforeach()
message(STATUS "Generated files will be in: ${PROTO_GEN_DIR}")
