include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_fw_selector)

target_sources(opendeck_bootloader_fw_selector PRIVATE
    fw_selector.cpp
)

target_include_directories(opendeck_bootloader_fw_selector
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../src
    ${CMAKE_CURRENT_LIST_DIR}/../../../firmware/src
)
