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
        ${PROJECT_ROOT}/modules
    )

    target_compile_definitions(application
        PRIVATE
        SW_VERSION_MAJOR=${SW_VERSION_MAJOR}
        SW_VERSION_MINOR=${SW_VERSION_MINOR}
        SW_VERSION_REVISION=${SW_VERSION_REVISION}
    )

    file(GLOB_RECURSE APP_GENERATED_SOURCES
        ${PROJECT_ROOT}/src/generated/application/${TARGET}/*.cpp
    )

    add_module(LESSDB)
    add_mcu_library(lessdb-lib)

    target_sources(application
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/io/buttons/Buttons.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/touchscreen/model/viewtech/Viewtech.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/touchscreen/model/Builder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/touchscreen/model/nextion/Nextion.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/touchscreen/Touchscreen.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/leds/LEDs.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/i2c/I2C.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/i2c/peripherals/display/Display.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/i2c/peripherals/display/strings/Strings.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/i2c/peripherals/Builder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/encoders/Encoders.cpp
        ${CMAKE_CURRENT_LIST_DIR}/io/analog/Analog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/protocol/midi/MIDI.cpp
        ${CMAKE_CURRENT_LIST_DIR}/database/CustomInit.cpp
        ${CMAKE_CURRENT_LIST_DIR}/database/Database.cpp
        ${CMAKE_CURRENT_LIST_DIR}/system/System.cpp
        ${CMAKE_CURRENT_LIST_DIR}/util/scheduler/Scheduler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/util/cinfo/CInfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/util/configurable/Configurable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${PROJECT_ROOT}/modules/midi/src/MIDI.cpp
        ${PROJECT_ROOT}/modules/midi/src/transport/ble/BLE.cpp
        ${PROJECT_ROOT}/modules/midi/src/transport/serial/Serial.cpp
        ${PROJECT_ROOT}/modules/midi/src/transport/usb/USB.cpp
        ${PROJECT_ROOT}/modules/sysex/src/SysExConf.cpp
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_string.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_setup.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_u8toa.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_8x8.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_u16toa.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_display.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_fonts.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_byte.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_cad.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_gpio.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c
        ${PROJECT_ROOT}/modules/u8g2/csrc/u8x8_d_ssd1306_128x32.c
        ${APP_GENERATED_SOURCES}
    )

    target_link_libraries(application
        PRIVATE
        ${BOARD_APPLICATION_LIBRARIES}
        lessdb-lib
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

    if (TARGET emueeprom-lib)
        set(FLASHGEN_BIN_CONF_NAME generated_config.hex)
        set(FLASHGEN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/flashgen)

        ExternalProject_Add(flashgen
            SOURCE_DIR      ${PROJECT_ROOT}/src/tools/flashgen
            CMAKE_ARGS      -DTARGET=${TARGET}
            CMAKE_ARGS      -DMCU_CMAKE_DISABLE_CORE_INCLUSION=1
            CMAKE_ARGS      -DPROJECT_MCU_FLASH_PAGE_FACTORY=${PROJECT_MCU_FLASH_PAGE_FACTORY}
            CMAKE_ARGS      -DEMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
            BINARY_DIR      ${FLASHGEN_BINARY_DIR}
            INSTALL_COMMAND "./flashgen" "${FLASHGEN_BIN_CONF_NAME}"
        )

        add_custom_command(
            DEPENDS application # ensure re-merge once application changes
            OUTPUT merged_app_with_flash_config # just an indicator
            COMMAND echo Merging application with generated flash config
            COMMAND srec_cat $<TARGET_FILE:application>.hex -Intel ${FLASHGEN_BINARY_DIR}/${FLASHGEN_BIN_CONF_NAME} -Intel -o $<TARGET_FILE:application>.hex -Intel
            COMMAND objcopy -I ihex $<TARGET_FILE:application>.hex --gap-fill 0xFF -O binary $<TARGET_FILE:application>.bin
            COMMAND touch merged_app_with_flash_config
        )

        add_custom_target(
            merge_app_with_flash_config
            ALL
            DEPENDS merged_app_with_flash_config
        )

        add_dependencies(merge_app_with_flash_config flashgen application)

        list(APPEND FIRMWARE_TARGETS merge_app_with_flash_config)
    endif()

    if (TARGET bootloader)
        set(SYSEXGEN_FW_NAME_RAW_SYSEX firmware.raw)
        set(SYSEXGEN_FW_NAME_ASCII_SYSEX firmware.sysex)
        set(SYSEX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/sysexgen)

        ExternalProject_Add(sysexgen
            SOURCE_DIR      ${PROJECT_ROOT}/src/tools/sysexgen
            CMAKE_ARGS      -DTARGET=${TARGET} -DMCU_CMAKE_DISABLE_CORE_INCLUSION=1
            BINARY_DIR      ${SYSEX_BINARY_DIR}
            INSTALL_COMMAND "./sysexgen" "$<TARGET_FILE:application>.bin" "${SYSEXGEN_FW_NAME_RAW_SYSEX}"
        )

        if (TARGET flashgen)
            add_dependencies(sysexgen flashgen)
        endif()

        add_custom_command(
            OUTPUT ${SYSEX_BINARY_DIR}/${SYSEXGEN_FW_NAME_ASCII_SYSEX}
            COMMAND hexdump -v -e \'/1 \"%02x \"\' ${SYSEX_BINARY_DIR}/${SYSEXGEN_FW_NAME_RAW_SYSEX} | sed 's\#f7\#f7\\n\#g' | sed 's\#^ *\#\#' | tr a-z A-Z > ${SYSEX_BINARY_DIR}/${SYSEXGEN_FW_NAME_ASCII_SYSEX}
        )

        add_custom_target(raw_sysex_to_ascii
            ALL
            DEPENDS ${SYSEX_BINARY_DIR}/${SYSEXGEN_FW_NAME_ASCII_SYSEX}
        )

        add_dependencies(raw_sysex_to_ascii sysexgen)

        list(APPEND FIRMWARE_TARGETS raw_sysex_to_ascii)
    endif()
endif()