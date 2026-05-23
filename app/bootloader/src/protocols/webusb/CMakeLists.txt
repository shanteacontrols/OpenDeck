include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_protocols_webusb)

target_sources(opendeck_bootloader_protocols_webusb PRIVATE
    instance/impl/webusb.cpp
)
