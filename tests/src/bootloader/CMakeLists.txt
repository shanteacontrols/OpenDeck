cmake_minimum_required(VERSION 3.20)
list(APPEND CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/prj.conf")
include($ENV{ZEPHYR_WS}/zenv/cmake/test.cmake)

project(test)

add_compile_definitions(
    OPENDECK_TEST
    PROJECT_MCU_SUPPORT_BOOTLOADER
    BOOTLOADER_TEST_FLASH_PAGE_SIZE=256
)

add_subdirectory($ENV{ZEPHYR_PROJECT}/app/src/firmware/application/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/src/tools/sysexgen/lib ${CMAKE_CURRENT_BINARY_DIR}/sysexgen_lib)

target_sources(app
    PRIVATE
    bootloader_test.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/bootloader/updater/updater.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/bootloader/sysex_parser/sysex_parser.cpp
)

target_include_directories(app
    PRIVATE
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

add_dependencies(app
    metadata
    sysexgen_lib
)

target_link_libraries(app
    PUBLIC
    metadata
    sysexgen_lib
)
