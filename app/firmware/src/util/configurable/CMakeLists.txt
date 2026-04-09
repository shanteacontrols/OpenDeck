include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(configurable)

target_include_directories(configurable
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/configurable.cpp
)
