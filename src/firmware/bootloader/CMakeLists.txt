if("PROJECT_MCU_SUPPORT_BOOTLOADER" IN_LIST PROJECT_MCU_DEFINES)
    add_executable(bootloader)
    list(APPEND FIRMWARE_TARGETS bootloader)

    target_include_directories(bootloader
        PRIVATE
        ${PROJECT_ROOT}/src/firmware
        ${PROJECT_ROOT}/modules
        ${PROJECT_ROOT}/modules/LESSDB/include
        ${PROJECT_ROOT}/modules/SysExConf/include
        ${PROJECT_ROOT}/modules/MIDI/include
    )

    target_sources(bootloader
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/FwSelector/FwSelector.cpp
        ${CMAKE_CURRENT_LIST_DIR}/updater/Updater.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SysExParser/SysExParser.cpp
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