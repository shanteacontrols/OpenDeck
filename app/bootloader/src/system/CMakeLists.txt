include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_system)

target_sources(opendeck_bootloader_system PRIVATE
    instance/impl/system.cpp
)

target_link_libraries(opendeck_bootloader_system
    PRIVATE
    zephyr_interface
)
