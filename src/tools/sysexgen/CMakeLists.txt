cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

project(sysexgen)
enable_language(CXX)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../)

include(${PROJECT_ROOT}/modules/core/cmake/ccache.cmake)
include(${PROJECT_ROOT}/src/generated/target/${TARGET}/CMakeLists.txt)

add_executable(sysexgen)

target_include_directories(sysexgen
    PRIVATE
    ${PROJECT_ROOT}/src/firmware
    ${PROJECT_ROOT}/modules/LESSDB/include
    ${PROJECT_ROOT}/modules/SysExConf/include
)

target_sources(sysexgen
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_compile_definitions(sysexgen
    PRIVATE
    ${PROJECT_TARGET_DEFINES}
)