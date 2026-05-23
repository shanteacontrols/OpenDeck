include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

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
    zlibs-utils-misc
    zlibs-utils-threads
    zephyr_interface
)
