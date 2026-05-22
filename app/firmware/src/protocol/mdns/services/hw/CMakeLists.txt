include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_mdns_services_hw)

target_link_libraries(protocol_mdns_services_hw
    PUBLIC
    opendeck_mdns
    protocol_mdns
    protocol_osc
    protocol_webconfig
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/services_hw.cpp
)
