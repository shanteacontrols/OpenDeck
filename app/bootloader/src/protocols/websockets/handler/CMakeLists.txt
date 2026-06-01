include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_bootloader_protocols_websockets_handler)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets/handler ${CMAKE_CURRENT_BINARY_DIR}/common_handler)
add_subdirectory(firmware_upload)

target_link_libraries(opendeck_bootloader_protocols_websockets_handler
    INTERFACE
    opendeck_common_protocols_websockets_handler
    opendeck_bootloader_protocols_websockets_handler_firmware_upload
)
