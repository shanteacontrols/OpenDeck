cmake_minimum_required(VERSION 3.20)
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap(
    APPLY_TARGET
    COMPILE_DEFINITIONS
    PROJECT_TARGET_HW_TESTS_SUPPORTED
    CONFIG_OPENDECK_LOG_LEVEL=3
)

add_subdirectory($ENV{ZEPHYR_PROJECT}/app/common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/firmware/src/database ${CMAKE_CURRENT_BINARY_DIR}/database)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/firmware/src/util/configurable ${CMAKE_CURRENT_BINARY_DIR}/configurable)

target_sources(app
    PRIVATE
    hw_test.cpp
)

add_dependencies(app
    metadata
    database
    configurable
)

target_link_libraries(app
    PUBLIC
    metadata
    database
    configurable
)
