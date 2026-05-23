include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_io_indicators)

target_sources(opendeck_bootloader_io_indicators PRIVATE
    instance/impl/indicators.cpp
)
