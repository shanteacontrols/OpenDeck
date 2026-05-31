include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_websockets)

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    target_link_libraries(opendeck_firmware_protocol_websockets
        PUBLIC
        kernel
        opendeck_common_mcu
    )

    if(CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE)
        add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/dfu/dfu_stream_parser ${CMAKE_CURRENT_BINARY_DIR}/dfu_stream_parser)
        add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets/firmware_upload ${CMAKE_CURRENT_BINARY_DIR}/websockets_firmware_upload)
        add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/firmware/src/dfu/staged_update_writer ${CMAKE_CURRENT_BINARY_DIR}/staged_update_writer)
    endif()

    add_subdirectory(handler)

    target_link_libraries(opendeck_firmware_protocol_websockets
        PUBLIC
        opendeck_firmware_protocol_websockets_handler
    )

    zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_LIST_DIR}/sections-rom.ld)

    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/websockets.cpp
    )
endif()
