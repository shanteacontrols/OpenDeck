target_compile_definitions(mcu
    PUBLIC
    CORE_MCU_BTLDR_CACHE_WORDS_USER=8192
)

list(APPEND BOARD_DEFINES
    BOARD_USE_TINYUSB
    BOARD_USE_UPDATE_HOOKS
    BOARD_USE_FAST_SOFT_PWM_TIMER
)

add_subdirectory(${PROJECT_ROOT}/modules/EmuEEPROM ${CMAKE_CURRENT_BINARY_DIR}/modules/EmuEEPROM)

target_compile_definitions(emueeprom-lib
    PUBLIC
    EMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
)

target_link_options(emueeprom-lib
    PUBLIC
    "$<TARGET_PROPERTY:mcu,LINK_OPTIONS>"
)

target_compile_options(emueeprom-lib
    PUBLIC
    "$<TARGET_PROPERTY:mcu,COMPILE_OPTIONS>"
)

# tinyusb
target_compile_definitions(usb
    PUBLIC
    CFG_TUD_MSC=0
    CFG_TUD_HID=0
    CFG_TUD_MIDI=1
    CFG_TUD_CDC=1
    CFG_TUD_VENDOR=0
)