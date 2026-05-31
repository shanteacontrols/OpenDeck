include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_protocols_webusb_hwa_hw)

target_sources(opendeck_bootloader_protocols_webusb_hwa_hw PRIVATE
    webusb_hw.cpp
)

target_link_libraries(opendeck_bootloader_protocols_webusb_hwa_hw
    PRIVATE
    opendeck_common_dfu_dfu_stream_parser
    opendeck_common_metadata
    zlibs_drivers_usb_hw
    zephyr_interface
)
