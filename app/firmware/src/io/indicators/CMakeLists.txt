include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(indicators)

target_include_directories(indicators
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/indicators.cpp
)
