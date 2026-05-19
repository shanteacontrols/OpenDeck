include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(protocol_midi_hwa_hw)

target_link_libraries(protocol_midi_hwa_hw
    INTERFACE
    zlibs-drivers-uart_hw
)

if(CONFIG_PROJECT_TARGET_SUPPORT_BLE)
    target_sources(protocol_midi_hwa_hw
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ble_service.cpp
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_USB_MIDI)
    target_link_libraries(protocol_midi_hwa_hw
        INTERFACE
        zlibs-drivers-usb_hw
    )
endif()
