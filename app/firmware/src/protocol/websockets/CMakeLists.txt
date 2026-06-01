include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_firmware_protocol_websockets)

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    if(CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE)
        add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/dfu/dfu_stream_parser ${CMAKE_CURRENT_BINARY_DIR}/dfu_stream_parser)
        add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets/firmware_upload ${CMAKE_CURRENT_BINARY_DIR}/websockets_firmware_upload)
        add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/firmware/src/dfu/staged_update_writer ${CMAKE_CURRENT_BINARY_DIR}/staged_update_writer)
    endif()

    add_subdirectory(handler)
    add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets ${CMAKE_CURRENT_BINARY_DIR}/common_websockets)

    target_link_libraries(opendeck_firmware_protocol_websockets
        INTERFACE
        opendeck_common_protocols_websockets
        opendeck_firmware_protocol_websockets_handler
    )

    if(CONFIG_HW_BUILD)
        add_subdirectory(hwa/hw)

        target_link_libraries(opendeck_firmware_protocol_websockets
            INTERFACE
            opendeck_firmware_protocol_websockets_hwa_hw
        )
    endif()

    zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_LIST_DIR}/sections-rom.ld)
endif()
