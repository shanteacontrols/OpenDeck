include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_osc_packet)


zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/packet.cpp
)
