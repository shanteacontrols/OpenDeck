include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_indicators)

target_sources(opendeck_bootloader_indicators PRIVATE
    instance/impl/indicators.cpp
)

