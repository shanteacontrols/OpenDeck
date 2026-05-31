include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_mdns_services_hw)

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/mdns/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/mdns_hwa_hw)

    target_link_libraries(opendeck_firmware_protocol_mdns_services_hw
        PUBLIC
        opendeck_common_protocols_mdns
        opendeck_common_protocols_mdns_hwa_hw
    )

    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/services_hw.cpp
    )

    if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
        zephyr_library_sources(
            ${CMAKE_CURRENT_LIST_DIR}/services_websockets.cpp
        )
    endif()

    if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
        zephyr_library_sources(
            ${CMAKE_CURRENT_LIST_DIR}/services_osc.cpp
        )
    endif()
endif()
