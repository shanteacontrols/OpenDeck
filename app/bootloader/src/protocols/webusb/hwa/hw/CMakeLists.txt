include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_bootloader_webusb_hwa_hw)

target_sources(opendeck_bootloader_webusb_hwa_hw PRIVATE
    webusb_hw.cpp
)

target_link_libraries(opendeck_bootloader_webusb_hwa_hw
    PRIVATE
    dfu_stream
    metadata
    zlibs-drivers-usb_hw
    zephyr_interface
)
