find_package(glog 0.6.0 REQUIRED)

include(${PROJECT_ROOT}/src/generated/target/${TARGET}/CMakeLists.txt)

add_library(common)

target_sources(common
    PRIVATE
    main.cpp
    ${PROJECT_ROOT}/modules/dbms/src/LESSDB.cpp
    ${PROJECT_ROOT}/modules/midi/src/MIDI.cpp
    ${PROJECT_ROOT}/modules/midi/src/transport/usb/USB.cpp
    ${PROJECT_ROOT}/modules/midi/src/transport/serial/Serial.cpp
    ${PROJECT_ROOT}/modules/midi/src/transport/ble/BLE.cpp
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
    ${PROJECT_ROOT}/src/firmware/application/util/configurable/Configurable.cpp
    ${PROJECT_ROOT}/src/firmware/application/util/scheduler/Scheduler.cpp
)

target_include_directories(common
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include
    ${PROJECT_ROOT}/modules/core/include
    ${PROJECT_ROOT}/modules/EmuEEPROM/include
    ${PROJECT_ROOT}/modules
    ${PROJECT_ROOT}/src/firmware
    ${PROJECT_ROOT}/src/firmware/application
    ${PROJECT_ROOT}/src/firmware/bootloader
    ${PROJECT_ROOT}/src/generated/target/${TARGET}
    ${PROJECT_ROOT}/src/generated/mcu/${PROJECT_TARGET_MCU}
    ${PROJECT_ROOT}/src/firmware/board/include
)

target_compile_definitions(common
    PUBLIC
    ${PROJECT_TARGET_DEFINES}
    ${PROJECT_MCU_DEFINES}
    TEST
    FW_APP
    APP_USE_LOGGER
    GLOG_CUSTOM_PREFIX_SUPPORT
)

target_link_libraries(common
    PUBLIC
    glog::glog
    gtest
    gmock
)

if("PROJECT_MCU_USE_EMU_EEPROM" IN_LIST PROJECT_MCU_DEFINES)
    add_subdirectory(${PROJECT_ROOT}/modules/EmuEEPROM ${CMAKE_CURRENT_BINARY_DIR}/modules/EmuEEPROM)

    target_compile_definitions(emueeprom
        PUBLIC
        EMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
    )

    target_link_libraries(common
        PUBLIC
        emueeprom
    )
endif()

add_subdirectory(bootloader)
add_subdirectory(database)
add_subdirectory(hw)
add_subdirectory(io)
add_subdirectory(protocol)
add_subdirectory(system)
add_subdirectory(usb_over_serial)