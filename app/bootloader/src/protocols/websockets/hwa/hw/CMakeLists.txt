include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_protocols_websockets_hwa_hw)

target_link_libraries(opendeck_bootloader_protocols_websockets_hwa_hw
    PUBLIC
    opendeck_common_protocols_websockets_hwa_hw
)

target_sources(opendeck_bootloader_protocols_websockets_hwa_hw PRIVATE
    hwa_hw.cpp
)
