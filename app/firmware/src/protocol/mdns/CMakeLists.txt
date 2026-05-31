include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_mdns)

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/mdns ${CMAKE_CURRENT_BINARY_DIR}/mdns)

    target_link_libraries(opendeck_firmware_protocol_mdns
        PUBLIC
        opendeck_firmware_database
        kernel
        opendeck_common_metadata
        opendeck_common_protocols_mdns
    )

    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/mdns.cpp
    )
endif()
