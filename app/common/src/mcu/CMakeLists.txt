include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_common_mcu)

target_link_libraries(opendeck_common_mcu
    INTERFACE
    zlibs-utils-misc
)
