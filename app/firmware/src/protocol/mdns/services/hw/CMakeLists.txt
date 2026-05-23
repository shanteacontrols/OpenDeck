include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_mdns_services_hw)

target_link_libraries(opendeck_firmware_protocol_mdns_services_hw
    PUBLIC
    opendeck_common_protocols_mdns
    opendeck_firmware_protocol_mdns
    opendeck_firmware_protocol_osc
    opendeck_firmware_protocol_websockets
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/services_hw.cpp
)
