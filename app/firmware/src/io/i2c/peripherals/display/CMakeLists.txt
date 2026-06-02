include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_i2c_peripherals_display TYPE OBJECT)

target_link_libraries(opendeck_firmware_io_i2c_peripherals_display
    PUBLIC
    opendeck_common_metadata
)

target_include_directories(opendeck_firmware_io_i2c_peripherals_display
    PUBLIC
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc
)

if(NOT BOARD MATCHES "^native_sim")
    target_compile_definitions(opendeck_firmware_io_i2c_peripherals_display
        PUBLIC
        U8X8_WITH_USER_PTR
    )
endif()

target_sources(opendeck_firmware_io_i2c_peripherals_display
    PRIVATE
    display.cpp
    elements.cpp
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
