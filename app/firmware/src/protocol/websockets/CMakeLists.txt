include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_websockets)

target_link_libraries(opendeck_firmware_protocol_websockets
    PUBLIC
    kernel
    opendeck_common_dfu_dfu_stream
    opendeck_common_mcu
    opendeck_common_protocols_websockets_firmware_upload
    opendeck_firmware_protocol_osc_packet
)

zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_LIST_DIR}/sections-rom.ld)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/websockets.cpp
)
