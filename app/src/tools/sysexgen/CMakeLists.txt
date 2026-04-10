cmake_minimum_required(VERSION 3.27.0)

project(sysexgen_host LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT TARGET metadata)
    add_subdirectory($ENV{ZEPHYR_PROJECT}/app/src/firmware/application/metadata metadata)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib sysexgen_lib)

add_executable(sysexgen_tool
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_link_libraries(sysexgen_tool
    PRIVATE
    metadata
    sysexgen_lib
)

if(DEFINED SYSEXGEN_INPUT_FILE AND DEFINED SYSEXGEN_OUTPUT_FILE)
    target_compile_definitions(sysexgen_tool
        PRIVATE
        SYSEXGEN_INPUT_FILE="${SYSEXGEN_INPUT_FILE}"
        SYSEXGEN_OUTPUT_FILE="${SYSEXGEN_OUTPUT_FILE}"
    )
endif()
