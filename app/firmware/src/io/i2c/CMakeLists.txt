include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_i2c)

target_include_directories(opendeck_firmware_io_i2c
    PUBLIC
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc
)

target_link_libraries(opendeck_firmware_io_i2c
    PUBLIC
    opendeck_common_metadata
    opendeck_firmware_util_configurable
    opendeck_firmware_database
)

if(NOT BOARD MATCHES "^native_sim")
    target_compile_definitions(opendeck_firmware_io_i2c
        PUBLIC
        U8X8_WITH_USER_PTR
    )
endif()

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/i2c.cpp
    ${CMAKE_CURRENT_LIST_DIR}/peripherals/display/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/peripherals/display/elements.cpp
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
)
