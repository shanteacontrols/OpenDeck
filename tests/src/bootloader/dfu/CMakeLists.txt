cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    COMPILE_DEFINITIONS
    BOOTLOADER_TEST_FLASH_SECTOR_SIZE=256
)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/dfu/writer ${CMAKE_CURRENT_BINARY_DIR}/writer)
add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/dfu/dfu_stream_parser ${CMAKE_CURRENT_BINARY_DIR}/dfu_stream_parser)

target_sources(app
    PRIVATE
    bootloader_dfu_test.cpp
    $ENV{ZENV_PROJECT_ROOT}/app/bootloader/src/io/indicators/instance/impl/indicators.cpp
    $ENV{ZENV_PROJECT_ROOT}/app/bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.cpp
    $ENV{ZENV_PROJECT_ROOT}/app/bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.cpp
)

add_dependencies(app
    opendeck_common_metadata
    opendeck_common_dfu_writer
    opendeck_common_dfu_dfu_stream_parser
)

target_link_libraries(app
    PUBLIC
    opendeck_common_metadata
    opendeck_common_dfu_writer
    opendeck_common_dfu_dfu_stream_parser
)
