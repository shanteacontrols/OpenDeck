include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_dfu_staged_update_reader)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/dfu/staged_update ${CMAKE_CURRENT_BINARY_DIR}/staged_update)

target_sources(opendeck_bootloader_dfu_staged_update_reader PRIVATE
    instance/impl/staged_update_reader.cpp
)

target_link_libraries(opendeck_bootloader_dfu_staged_update_reader
    PRIVATE
    opendeck_common_dfu_staged_update
    opendeck_common_dfu_dfu_stream_parser
    opendeck_common_metadata
    zephyr_interface
)
