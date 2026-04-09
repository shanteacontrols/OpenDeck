include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_updater)

target_sources(opendeck_bootloader_updater PRIVATE
    updater.cpp
)

target_include_directories(opendeck_bootloader_updater
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../src
    ${CMAKE_CURRENT_LIST_DIR}/../../../firmware/src
)

target_link_libraries(opendeck_bootloader_updater
    PRIVATE
    metadata
    zephyr_interface
)
