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
)

target_sources(app
    PRIVATE
    analog_filter_test.cpp
)

target_include_directories(app
    PRIVATE
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)
