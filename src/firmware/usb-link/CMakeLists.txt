if("PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(usb_link)
    list(APPEND FIRMWARE_TARGETS usb_link)

    target_include_directories(usb_link
        PRIVATE
        ${PROJECT_ROOT}/src/firmware
        ${WORKSPACE_ROOT}/modules/liblessdb/include
        ${WORKSPACE_ROOT}/modules/libsysexconf/include
        ${WORKSPACE_ROOT}/modules/libmidi/include
    )

    target_sources(usb_link
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    )

    target_link_libraries(usb_link ${BOARD_APPLICATION_LIBRARIES})

    add_custom_command(
        TARGET usb_link
        POST_BUILD
        COMMAND objcopy --gap-fill 0xFF -O ihex $<TARGET_FILE:usb_link> $<TARGET_FILE:usb_link>.hex
    )

    list(APPEND FIRMWARE_HEX_LIST
        $<TARGET_FILE:usb_link>.hex
    )
endif()