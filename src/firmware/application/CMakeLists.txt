# USB-over-serial hosts don't execute application
if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(application)
    list(APPEND FIRMWARE_TARGETS application)

    execute_process(
        COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1"
        OUTPUT_VARIABLE SW_VERSION_MAJOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2"
        OUTPUT_VARIABLE SW_VERSION_MINOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3"
        OUTPUT_VARIABLE SW_VERSION_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    target_include_directories(application
        PRIVATE
        ${PROJECT_ROOT}/src/firmware
        ${WORKSPACE_ROOT}/modules/u8g2/csrc
    )

    target_compile_definitions(application
        PRIVATE
        SW_VERSION_MAJOR=${SW_VERSION_MAJOR}
        SW_VERSION_MINOR=${SW_VERSION_MINOR}
        SW_VERSION_REVISION=${SW_VERSION_REVISION}
        U8X8_WITH_USER_PTR
    )

    file(GLOB_RECURSE APP_GENERATED_SOURCES
        ${PROJECT_ROOT}/src/generated/application/${TARGET}/*.cpp
    )

    add_module(liblessdb)
    add_mcu_library(liblessdb)

    add_module(libsysexconf)
    add_mcu_library(libsysexconf)

    add_module(libmidi)
    add_mcu_library(libmidi)

    target_sources(application
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/io/buttons/buttons.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/touchscreen/models/viewtech/viewtech.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/touchscreen/models/nextion/nextion.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/touchscreen/touchscreen.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/leds/leds.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/i2c/i2c.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/i2c/peripherals/display/display.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/i2c/peripherals/display/elements.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/encoders/encoders.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/analog/analog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/protocol/midi/midi.cpp
        ${CMAKE_CURRENT_LIST_DIR}/database/custom_init.cpp
        ${CMAKE_CURRENT_LIST_DIR}/database/database.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system/system.cpp
        ${CMAKE_CURRENT_LIST_DIR}/util/scheduler/scheduler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/util/cinfo/cinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/util/configurable/configurable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_string.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_setup.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_u8toa.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_8x8.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_u16toa.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_display.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_fonts.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_byte.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_cad.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_gpio.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c
        ${WORKSPACE_ROOT}/modules/u8g2/csrc/u8x8_d_ssd1306_128x32.c
        ${APP_GENERATED_SOURCES}
    )

    target_link_libraries(application
        PRIVATE
        ${BOARD_APPLICATION_LIBRARIES}
        liblessdb
        libsysexconf
        libmidi
    )

    add_custom_command(
        TARGET application
        POST_BUILD
        COMMAND echo Converting elf file to hex
        COMMAND objcopy --gap-fill 0xFF -O ihex $<TARGET_FILE:application> $<TARGET_FILE:application>.hex
        COMMAND echo Appending firmware metadata to hex file
        COMMAND srec_cat $<TARGET_FILE:application>.hex -Intel -exclude ${PROJECT_MCU_FLASH_ADDR_FW_METADATA_START} ${PROJECT_MCU_FLASH_ADDR_FW_METADATA_END} -MAximum_Little_Endian ${PROJECT_MCU_FLASH_ADDR_FW_METADATA_START} -o $<TARGET_FILE:application>.hex -Intel
        COMMAND echo Adding firmware CRC at the end of application flash
        COMMAND srec_cat $<TARGET_FILE:application>.hex -Intel -Cyclic_Redundancy_Check_16_Little_Endian -MAximum-Address $<TARGET_FILE:application>.hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $<TARGET_FILE:application>.hex -Intel
        COMMAND echo Converting hex file to bin
        COMMAND objcopy -I ihex -O binary $<TARGET_FILE:application>.hex $<TARGET_FILE:application>.bin
    )

    list(APPEND FIRMWARE_HEX_LIST
        $<TARGET_FILE:application>.hex
    )

    if (TARGET libemueeprom)
        set(FLASHGEN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/flashgen)
        set(FLASHGEN_BINARY ${FLASHGEN_BINARY_DIR}/flashgen)
        set(FLASHGEN_OUTPUT ${FLASHGEN_BINARY_DIR}/generated_config.hex)

        ExternalProject_Add(flashgen
            SOURCE_DIR      ${PROJECT_ROOT}/src/tools/flashgen
            CMAKE_ARGS      -DTARGET=${TARGET}
            CMAKE_ARGS      -DCMAKE_USE_STUB_MCU=1
            CMAKE_ARGS      -DPROJECT_MCU_FLASH_PAGE_FACTORY=${PROJECT_MCU_FLASH_PAGE_FACTORY}
            CMAKE_ARGS      -DEMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
            BINARY_DIR      ${FLASHGEN_BINARY_DIR}
            BUILD_ALWAYS    TRUE
            INSTALL_COMMAND ""
        )

        add_custom_command(
            DEPENDS flashgen
            OUTPUT ${FLASHGEN_OUTPUT}
            COMMAND ${FLASHGEN_BINARY} ${FLASHGEN_OUTPUT}
        )

        add_custom_target(
            generate_flash_config
            DEPENDS ${FLASHGEN_OUTPUT}
        )

        add_custom_command(
            DEPENDS application # ensure re-merge once application changes
            DEPENDS generate_flash_config
            OUTPUT merged_app_with_flash_config # just an indicator
            COMMAND echo Merging application with generated flash config
            COMMAND srec_cat $<TARGET_FILE:application>.hex -Intel ${FLASHGEN_OUTPUT} -Intel -o $<TARGET_FILE:application>.hex -Intel
            COMMAND objcopy -I ihex $<TARGET_FILE:application>.hex --gap-fill 0xFF -O binary $<TARGET_FILE:application>.bin
            COMMAND touch merged_app_with_flash_config
        )

        add_custom_target(
            merge_app_with_flash_config
            DEPENDS merged_app_with_flash_config
        )

        list(APPEND FIRMWARE_TARGETS merge_app_with_flash_config)
    endif()

    if (TARGET bootloader)
        set(SYSEX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/sysexgen)
        set(SYSEX_BINARY ${SYSEX_BINARY_DIR}/sysexgen)
        set(SYSEXGEN_RAW_OUTPUT ${SYSEX_BINARY_DIR}/firmware.raw)
        set(SYSEXGEN_ASCII_OUTPUT ${SYSEX_BINARY_DIR}/firmware.sysex)

        ExternalProject_Add(sysexgen
            SOURCE_DIR      ${PROJECT_ROOT}/src/tools/sysexgen
            CMAKE_ARGS      -DTARGET=${TARGET} -DCMAKE_USE_STUB_MCU=1
            BINARY_DIR      ${SYSEX_BINARY_DIR}
            BUILD_ALWAYS    TRUE
            INSTALL_COMMAND ""
        )

        add_custom_command(
            DEPENDS sysexgen
            OUTPUT ${SYSEXGEN_RAW_OUTPUT}
            COMMAND ${SYSEX_BINARY} "$<TARGET_FILE:application>.bin" "${SYSEXGEN_RAW_OUTPUT}"
        )

        add_custom_command(
            DEPENDS ${SYSEXGEN_RAW_OUTPUT}
            OUTPUT ${SYSEXGEN_ASCII_OUTPUT}
            COMMAND hexdump -v -e \'/1 \"%02x \"\' ${SYSEXGEN_RAW_OUTPUT} | sed 's\#f7\#f7\\n\#g' | sed 's\#^ *\#\#' | tr a-z A-Z > ${SYSEXGEN_ASCII_OUTPUT}
        )

        add_custom_target(
            generate_sysex_firmware
            DEPENDS application
            DEPENDS ${SYSEXGEN_ASCII_OUTPUT}
        )

        if (TARGET flashgen)
            add_dependencies(generate_sysex_firmware merge_app_with_flash_config)
        endif()

        list(APPEND FIRMWARE_TARGETS generate_sysex_firmware)
    endif()
endif()