include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(touchscreen_hwa_hw)

target_link_libraries(touchscreen_hwa_hw
    INTERFACE
    zlibs-drivers-uart_hw
)
