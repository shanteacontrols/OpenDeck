include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_direct_update_writer)

target_sources(opendeck_bootloader_direct_update_writer PRIVATE
    instance/impl/direct_update_writer.cpp
)

target_link_libraries(opendeck_bootloader_direct_update_writer
    PRIVATE
    dfu_stream
    flash_stream_writer
    metadata
    zephyr_interface
)
