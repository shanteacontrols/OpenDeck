find_package(glog 0.6.0 REQUIRED)

include($ENV{ZEPHYR_PROJECT}/app/src/generated/target/${TARGET}/CMakeLists.txt)

add_library(common)

add_module(liblessdb)
add_module(libsysexconf)
add_module(libmidi)

target_sources(common
    PRIVATE
    main.cpp
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_string.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_setup.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_u8toa.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_8x8.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_u16toa.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_display.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_fonts.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_byte.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_cad.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_gpio.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc/u8x8_d_ssd1306_128x32.c
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/util/configurable/configurable.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/util/scheduler/scheduler.cpp
)

target_include_directories(common
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include
    $ENV{ZEPHYR_WS}/modules/libcore/include
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/bootloader
    $ENV{ZEPHYR_PROJECT}/app/src/generated/target/${TARGET}
    $ENV{ZEPHYR_PROJECT}/app/src/generated/mcu/${PROJECT_TARGET_MCU}
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/board/include
)

target_compile_definitions(common
    PUBLIC
    ${PROJECT_TARGET_DEFINES}
    ${PROJECT_MCU_DEFINES}
    OPENDECK_TEST
    OPENDECK_FW_APP
    OPENDECK_USE_LOGGER
    GLOG_CUSTOM_PREFIX_SUPPORT
)

target_link_libraries(common
    PUBLIC
    glog::glog
    gtest
    gmock
    liblessdb
    libsysexconf
    libmidi
    mcu
)

if("PROJECT_MCU_USE_EMU_EEPROM" IN_LIST PROJECT_MCU_DEFINES)
    add_module(libemueeprom)

    target_compile_definitions(libemueeprom
        PUBLIC
        EMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
    )

    target_link_libraries(common
        PUBLIC
        libemueeprom
    )
endif()

add_subdirectory(bootloader)
add_subdirectory(database)
add_subdirectory(hw)
add_subdirectory(io)
add_subdirectory(protocol)
add_subdirectory(system)
add_subdirectory(usb_over_serial)