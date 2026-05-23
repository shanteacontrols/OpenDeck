include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_mdns)

target_link_libraries(opendeck_firmware_protocol_mdns
    PUBLIC
    opendeck_firmware_database
    kernel
    opendeck_common_metadata
    opendeck_common_protocols_mdns
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/mdns.cpp
)
