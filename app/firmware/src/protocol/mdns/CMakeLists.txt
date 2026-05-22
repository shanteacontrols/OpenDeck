include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(protocol_mdns)

target_link_libraries(protocol_mdns
    PUBLIC
    database
    kernel
    metadata
    opendeck_mdns
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/mdns.cpp
)
