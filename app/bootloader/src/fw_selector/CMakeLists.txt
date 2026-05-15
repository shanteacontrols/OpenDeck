include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_fw_selector)

target_sources(opendeck_bootloader_fw_selector PRIVATE
    fw_selector.cpp
)

