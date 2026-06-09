include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_websockets_handler_firmware_upload)

target_link_libraries(opendeck_firmware_protocol_websockets_handler_firmware_upload
    PUBLIC
    opendeck_common_dfu_upload
    opendeck_firmware_dfu_staged_update_writer
    kernel
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/firmware_upload_handler.cpp
)
