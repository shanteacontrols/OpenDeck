include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_firmware_io_touchscreen_hwa_hw)

target_link_libraries(opendeck_firmware_io_touchscreen_hwa_hw
    INTERFACE
    zlibs_drivers_uart_hw
)
