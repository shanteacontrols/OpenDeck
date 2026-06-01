include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_protocols_mdns)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/mdns ${CMAKE_CURRENT_BINARY_DIR}/mdns)

target_link_libraries(opendeck_bootloader_protocols_mdns
    PUBLIC
    opendeck_common_protocols_mdns
)

if(CONFIG_HW_BUILD)
    add_subdirectory(services/hw)

    target_link_libraries(opendeck_bootloader_protocols_mdns
        PUBLIC
        opendeck_bootloader_protocols_mdns_services_hw
    )
endif()

target_sources(opendeck_bootloader_protocols_mdns PRIVATE
    instance/impl/mdns.cpp
)
