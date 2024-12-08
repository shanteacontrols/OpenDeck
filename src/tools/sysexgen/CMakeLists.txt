cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

project(sysexgen)
enable_language(CXX)

set(WORKSPACE_ROOT $ENV{WORKSPACE_DIR})
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../)

include(${WORKSPACE_ROOT}/modules/libcore/cmake/ccache.cmake)
include(${PROJECT_ROOT}/src/generated/target/${TARGET}/CMakeLists.txt)

add_executable(sysexgen)

target_include_directories(sysexgen
    PRIVATE
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
)

target_sources(sysexgen
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_compile_definitions(sysexgen
    PRIVATE
    ${PROJECT_TARGET_DEFINES}
)