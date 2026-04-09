include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_indicators)

target_sources(opendeck_bootloader_indicators PRIVATE
    indicators.cpp
)

target_include_directories(opendeck_bootloader_indicators
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../src
    ${CMAKE_CURRENT_LIST_DIR}/../../../firmware/src
)
