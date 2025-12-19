if("PROJECT_MCU_SUPPORT_BOOTLOADER" IN_LIST PROJECT_MCU_DEFINES)
    add_executable(bootloader)
    list(APPEND FIRMWARE_TARGETS bootloader)

    target_include_directories(bootloader
        PRIVATE
        ${PROJECT_ROOT}/src/firmware
        ${WORKSPACE_ROOT}/modules/libsysexconf/include
        ${WORKSPACE_ROOT}/modules/libmidi/include
    )

    target_sources(bootloader
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/fw_selector/fw_selector.cpp
        ${CMAKE_CURRENT_LIST_DIR}/updater/updater.cpp
        ${CMAKE_CURRENT_LIST_DIR}/sysex_parser/sysex_parser.cpp
    )

    target_link_libraries(bootloader ${BOARD_BOOTLOADER_LIBRARIES})

    add_custom_command(
        TARGET bootloader
        POST_BUILD
        COMMAND objcopy --gap-fill 0xFF -O ihex $<TARGET_FILE:bootloader> $<TARGET_FILE:bootloader>.hex
    )

    list(APPEND FIRMWARE_HEX_LIST
        $<TARGET_FILE:bootloader>.hex
    )
endif()