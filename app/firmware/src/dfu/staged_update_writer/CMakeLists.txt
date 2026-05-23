include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_dfu_staged_update_writer)

target_link_libraries(opendeck_firmware_dfu_staged_update_writer
    PUBLIC
    opendeck_common_dfu_flash_stream_writer
    opendeck_common_metadata
    kernel
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/staged_update_writer.cpp
)
