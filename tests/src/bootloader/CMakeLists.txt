if("PROJECT_MCU_SUPPORT_BOOTLOADER" IN_LIST PROJECT_MCU_DEFINES)
    add_executable(bootloader)

    target_sources(bootloader
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/bootloader/sysex_parser/sysex_parser.cpp
        ${PROJECT_ROOT}/src/firmware/bootloader/updater/updater.cpp
    )

    target_link_libraries(bootloader
        PUBLIC
        common
    )

    add_custom_command(
        TARGET bootloader
        POST_BUILD
        COMMAND cp ${FW_BUILD_DIR}/sysexgen/firmware.raw ${CMAKE_CURRENT_BINARY_DIR}/firmware.syx
        COMMAND cp ${FW_BUILD_DIR}/application.elf.bin ${CMAKE_CURRENT_BINARY_DIR}/firmware.bin
    )

    add_test(
        NAME bootloader
        COMMAND $<TARGET_FILE:bootloader>
    )
endif()