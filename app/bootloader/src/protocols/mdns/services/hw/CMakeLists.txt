include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_protocols_mdns_services_hw)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/mdns/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/mdns_hwa_hw)

target_link_libraries(opendeck_bootloader_protocols_mdns_services_hw
    PUBLIC
    opendeck_common_protocols_mdns
    opendeck_common_protocols_mdns_hwa_hw
)

target_sources(opendeck_bootloader_protocols_mdns_services_hw PRIVATE
    services_hw.cpp
)
