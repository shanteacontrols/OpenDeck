include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_protocols_mdns)

target_link_libraries(opendeck_bootloader_protocols_mdns
    PUBLIC
    opendeck_common_protocols_mdns
)

target_sources(opendeck_bootloader_protocols_mdns PRIVATE
    instance/impl/mdns.cpp
)
