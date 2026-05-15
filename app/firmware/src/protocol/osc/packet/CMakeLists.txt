include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_osc_packet)


zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/packet.cpp
)
