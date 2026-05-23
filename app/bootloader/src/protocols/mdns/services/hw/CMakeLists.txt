include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_protocols_mdns_services_hw)

target_link_libraries(opendeck_bootloader_protocols_mdns_services_hw
    PUBLIC
    opendeck_common_protocols_mdns
)

target_sources(opendeck_bootloader_protocols_mdns_services_hw PRIVATE
    services_hw.cpp
)
