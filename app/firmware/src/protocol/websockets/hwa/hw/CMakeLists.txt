include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_websockets_hwa_hw)

target_link_libraries(opendeck_firmware_protocol_websockets_hwa_hw
    PUBLIC
    opendeck_common_mcu
    opendeck_common_protocols_websockets_hwa_hw
    opendeck_firmware_protocol_websockets
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/hwa_hw.cpp
)
