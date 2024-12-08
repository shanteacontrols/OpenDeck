if("PROJECT_TARGET_USB_OVER_SERIAL" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(usb_over_serial)

    target_sources(usb_over_serial
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/board/src/common/communication/usb_over_serial/usb_over_serial.cpp
    )

    target_link_libraries(usb_over_serial
        PUBLIC
        common
    )

    add_test(
        NAME usb_over_serial
        COMMAND $<TARGET_FILE:usb_over_serial>
    )
endif()