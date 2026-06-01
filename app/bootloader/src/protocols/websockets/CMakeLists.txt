include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_bootloader_protocols_websockets)

zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_LIST_DIR}/sections-rom.ld)

add_subdirectory(handler)
add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets ${CMAKE_CURRENT_BINARY_DIR}/common_websockets)
add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets/firmware_upload ${CMAKE_CURRENT_BINARY_DIR}/websockets_firmware_upload)

target_link_libraries(opendeck_bootloader_protocols_websockets
    INTERFACE
    opendeck_common_protocols_websockets
    opendeck_bootloader_protocols_websockets_handler
)

if(CONFIG_HW_BUILD)
    add_subdirectory(hwa/hw)

    target_link_libraries(opendeck_bootloader_protocols_websockets
        INTERFACE
        opendeck_bootloader_protocols_websockets_hwa_hw
    )
endif()
