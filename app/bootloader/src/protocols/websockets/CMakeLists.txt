include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_protocols_websockets)

zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_LIST_DIR}/sections-rom.ld)

target_sources(opendeck_bootloader_protocols_websockets PRIVATE
    instance/impl/websockets.cpp
)

target_link_libraries(opendeck_bootloader_protocols_websockets
    PRIVATE
    opendeck_common_dfu_dfu_stream
    opendeck_bootloader_dfu_direct_update_writer
    opendeck_common_protocols_websockets_firmware_upload
    zlibs_utils_misc
    zlibs_utils_threads
    zephyr_interface
)
