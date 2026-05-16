include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_staged_update)

target_sources(opendeck_bootloader_staged_update PRIVATE
    instance/impl/staged_update.cpp
)


target_link_libraries(opendeck_bootloader_staged_update
    PRIVATE
    metadata
    zephyr_interface
    opendeck_bootloader_indicators
    opendeck_bootloader_updater
)
