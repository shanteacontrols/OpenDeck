include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_osc)

add_subdirectory(packet)

target_link_libraries(opendeck_firmware_protocol_osc
    PUBLIC
    kernel
    opendeck_common_metadata
    opendeck_firmware_protocol_osc_packet
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/osc.cpp
)
