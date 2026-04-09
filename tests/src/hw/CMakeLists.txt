cmake_minimum_required(VERSION 3.20)
set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)
list(APPEND CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/prj.conf")
list(APPEND DTS_ROOT "$ENV{ZEPHYR_PROJECT}/app")
include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_target.cmake)
opendeck_test_apply_target()
include($ENV{ZEPHYR_WS}/zenv/cmake/test.cmake)

project(test)

add_compile_definitions(
    OPENDECK_TEST
    PROJECT_TARGET_HW_TESTS_SUPPORTED
    CONFIG_OPENDECK_LOG_LEVEL=3
)

add_subdirectory($ENV{ZEPHYR_PROJECT}/app/common/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/firmware/src/database ${CMAKE_CURRENT_BINARY_DIR}/database)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/firmware/src/util/configurable ${CMAKE_CURRENT_BINARY_DIR}/configurable)

target_sources(app
    PRIVATE
    hw_test.cpp
)

target_include_directories(app
    PRIVATE
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src
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
