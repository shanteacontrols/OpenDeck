cmake_minimum_required(VERSION 3.20)
list(APPEND CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/prj.conf")
include($ENV{ZEPHYR_WS}/zenv/cmake/test.cmake)

project(test)

add_compile_definitions(
    OPENDECK_TEST
)

add_subdirectory($ENV{ZEPHYR_PROJECT}/app/src/firmware/application/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database ${CMAKE_CURRENT_BINARY_DIR}/database)
add_subdirectory($ENV{ZEPHYR_PROJECT}/app/src/firmware/application/util/configurable ${CMAKE_CURRENT_BINARY_DIR}/configurable)

target_sources(app
    PRIVATE
    database_test.cpp
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
