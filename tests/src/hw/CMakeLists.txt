cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    IMPORT_TARGET_CONFIG
    COMPILE_DEFINITIONS
    PROJECT_TARGET_HW_TESTS_SUPPORTED
    CONFIG_OPENDECK_LOG_LEVEL=3
)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/firmware/src/database ${CMAKE_CURRENT_BINARY_DIR}/database)
add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/firmware/src/util/configurable ${CMAKE_CURRENT_BINARY_DIR}/configurable)

target_sources(app
    PRIVATE
    hw_test.cpp
)

add_dependencies(app
    opendeck_common_metadata
    opendeck_firmware_database
    opendeck_firmware_util_configurable
)

target_link_libraries(app
    PUBLIC
    opendeck_common_metadata
    opendeck_firmware_database
    opendeck_firmware_util_configurable
)
