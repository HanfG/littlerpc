cmake_minimum_required(VERSION 3.0.0)

project(LittleRPC VERSION 0.1.0)

set(PROTOBUF_C_PATH "${PROJECT_SOURCE_DIR}/extension/protobuf-c")

option(BUILD_PROTOC OFF)
add_subdirectory("${PROTOBUF_C_PATH}/build-cmake")

set(SOURCES
    "${PROJECT_SOURCE_DIR}/littlerpc/littlerpc.c"
    "${PROJECT_SOURCE_DIR}/littlerpc/littlerpc_seq_callback_manager.c"
    "${PROJECT_SOURCE_DIR}/littlerpc/littlerpc_pbc_service_manager.c")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROTOBUF_C_PATH})

add_library(LittleRPC ${SOURCES})

if(BUILD_EXAMPLE)
    add_subdirectory("${PROJECT_SOURCE_DIR}/example")
endif()

target_link_libraries(LittleRPC protobuf-c)
