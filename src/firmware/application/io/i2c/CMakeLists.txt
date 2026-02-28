add_library(i2c)

target_sources(i2c
    PRIVATE
    peripherals/display/display.cpp
    peripherals/display/elements.cpp
    i2c.cpp
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
)

target_include_directories(i2c
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
    ${WORKSPACE_ROOT}/modules/u8g2/csrc
)

target_compile_definitions(i2c
    PUBLIC
    U8X8_WITH_USER_PTR
)

add_dependencies(i2c
    board_application
    version
    database
)

target_link_libraries(i2c
    PUBLIC
    board_application
    libmidi
    version
    database
)