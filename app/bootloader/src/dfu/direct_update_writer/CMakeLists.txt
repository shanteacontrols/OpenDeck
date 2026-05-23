include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_dfu_direct_update_writer)

target_sources(opendeck_bootloader_dfu_direct_update_writer PRIVATE
    instance/impl/direct_update_writer.cpp
)

target_link_libraries(opendeck_bootloader_dfu_direct_update_writer
    PRIVATE
    opendeck_common_dfu_dfu_stream
    opendeck_common_dfu_flash_stream_writer
    opendeck_common_metadata
    zephyr_interface
)
