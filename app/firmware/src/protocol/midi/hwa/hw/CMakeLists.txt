include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_firmware_protocol_midi_hwa_hw)

if(CONFIG_PROJECT_TARGET_SUPPORT_BLE)
    target_sources(opendeck_firmware_protocol_midi_hwa_hw
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ble_service.cpp
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI)
    target_link_libraries(opendeck_firmware_protocol_midi_hwa_hw
        INTERFACE
        zlibs_drivers_uart_hw
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_USB_MIDI)
    target_link_libraries(opendeck_firmware_protocol_midi_hwa_hw
        INTERFACE
        zlibs_drivers_usb_hw
    )
endif()
