include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_protocols_webusb)

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU)
    target_sources(opendeck_bootloader_protocols_webusb PRIVATE
        instance/impl/webusb.cpp
    )

    if(CONFIG_HW_BUILD)
        add_subdirectory(hwa/hw)

        target_link_libraries(opendeck_bootloader_protocols_webusb
            PUBLIC
            opendeck_bootloader_protocols_webusb_hwa_hw
        )
    endif()
endif()
