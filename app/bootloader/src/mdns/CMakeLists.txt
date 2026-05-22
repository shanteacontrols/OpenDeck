include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_mdns)

target_link_libraries(opendeck_bootloader_mdns
    PUBLIC
    opendeck_mdns
)

target_sources(opendeck_bootloader_mdns PRIVATE
    instance/impl/mdns.cpp
)
