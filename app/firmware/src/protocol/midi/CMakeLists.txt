include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_midi)


target_link_libraries(protocol_midi
    PUBLIC
    database
    configurable
    zlibs-utils-midi
    kernel
)

if(NOT OPENDECK_TEST)
    target_link_libraries(protocol_midi
        PUBLIC
        zlibs-drivers-uart_hw
    )

    if(CONFIG_PROJECT_TARGET_SUPPORT_USB_MIDI)
        target_link_libraries(protocol_midi
            PUBLIC
            zlibs-drivers-usb_hw
        )
    endif()
endif()

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/midi.cpp
)

zephyr_library_sources_ifdef(
    CONFIG_PROJECT_TARGET_SUPPORT_BLE
    ${CMAKE_CURRENT_LIST_DIR}/ble_service.cpp
)
