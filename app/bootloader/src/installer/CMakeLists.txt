include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_installer)

target_sources(opendeck_bootloader_installer PRIVATE
    instance/impl/installer.cpp
)

target_link_libraries(opendeck_bootloader_installer
    PRIVATE
    flash_stream_writer
    metadata
    zephyr_interface
)
