include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(i2c)

target_include_directories(i2c
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc
)

target_link_libraries(i2c
    PUBLIC
    metadata
    configurable
    database
)

if(NOT BOARD MATCHES "^native_sim")
    target_compile_definitions(i2c
        PUBLIC
        U8X8_WITH_USER_PTR
    )
endif()

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/i2c.cpp
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
