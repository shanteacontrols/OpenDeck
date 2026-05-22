include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(protocol_websockets)

target_link_libraries(protocol_websockets
    PUBLIC
    kernel
    dfu_stream
    mcu
    opendeck_websockets_firmware_upload
    protocol_osc_packet
)

zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_LIST_DIR}/sections-rom.ld)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/websockets.cpp
)
