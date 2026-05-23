include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_dfu_staged_update_reader)

target_sources(opendeck_bootloader_dfu_staged_update_reader PRIVATE
    instance/impl/staged_update_reader.cpp
)

target_link_libraries(opendeck_bootloader_dfu_staged_update_reader
    PRIVATE
    opendeck_common_dfu_dfu_stream
    opendeck_common_metadata
    zephyr_interface
)
