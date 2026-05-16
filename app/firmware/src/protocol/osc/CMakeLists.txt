include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
add_subdirectory(packet)

opendeck_library_named(protocol_osc)

target_link_libraries(protocol_osc
    PUBLIC
    kernel
    metadata
    protocol_osc_packet
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/osc.cpp
)
