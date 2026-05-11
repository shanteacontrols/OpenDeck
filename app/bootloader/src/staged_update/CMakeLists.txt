include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_staged_update)

target_sources(opendeck_bootloader_staged_update PRIVATE
    staged_update.cpp
)

target_include_directories(opendeck_bootloader_staged_update
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../src
    ${CMAKE_CURRENT_LIST_DIR}/../indicators
    ${CMAKE_CURRENT_LIST_DIR}/../../../common
    ${CMAKE_CURRENT_LIST_DIR}/../../../firmware/src
)

target_link_libraries(opendeck_bootloader_staged_update
    PRIVATE
    metadata
    zephyr_interface
    opendeck_bootloader_indicators
    opendeck_bootloader_updater
)
