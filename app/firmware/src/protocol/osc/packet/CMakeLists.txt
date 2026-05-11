include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_osc_packet)

target_include_directories(protocol_osc_packet
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/packet.cpp
)
