include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_firmware_protocol_websockets_handler)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets/handler ${CMAKE_CURRENT_BINARY_DIR}/common_handler)

target_link_libraries(opendeck_firmware_protocol_websockets_handler
    INTERFACE
    opendeck_common_protocols_websockets_handler
)

if(CONFIG_PROJECT_TARGET_SUPPORT_CONFIG_INTERFACE_WEBSOCKETS)
    add_subdirectory(sysex_config)

    target_link_libraries(opendeck_firmware_protocol_websockets_handler
        INTERFACE
        opendeck_firmware_protocol_websockets_handler_sysex_config
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE)
    add_subdirectory(firmware_upload)

    target_link_libraries(opendeck_firmware_protocol_websockets_handler
        INTERFACE
        opendeck_firmware_protocol_websockets_handler_firmware_upload
    )
endif()
