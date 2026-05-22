include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_mdns_services_hw)

target_link_libraries(opendeck_bootloader_mdns_services_hw
    PUBLIC
    opendeck_mdns
)

target_sources(opendeck_bootloader_mdns_services_hw PRIVATE
    services_hw.cpp
)
