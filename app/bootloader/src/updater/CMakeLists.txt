include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_updater)

target_sources(opendeck_bootloader_updater PRIVATE
    updater.cpp
)


target_link_libraries(opendeck_bootloader_updater
    PRIVATE
    metadata
    zephyr_interface
)
