include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(mcu)

target_link_libraries(mcu
    INTERFACE
    zlibs-utils-misc
)
