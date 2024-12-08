find_package(glog 0.6.0 REQUIRED)

include(${PROJECT_ROOT}/src/generated/target/${TARGET}/CMakeLists.txt)

add_library(common)

add_module(liblessdb)
add_module(libsysexconf)
add_module(libmidi)

target_sources(common
    PRIVATE
    main.cpp
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
    ${PROJECT_ROOT}/src/firmware/application/util/configurable/configurable.cpp
    ${PROJECT_ROOT}/src/firmware/application/util/scheduler/scheduler.cpp
)

target_include_directories(common
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include
    ${WORKSPACE_ROOT}/modules/libcore/include
    ${WORKSPACE_ROOT}/modules/u8g2/csrc
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