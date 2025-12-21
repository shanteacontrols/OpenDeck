target_compile_definitions(mcu
    PUBLIC
    CORE_MCU_BTLDR_CACHE_WORDS_USER=8192
)

list(APPEND BOARD_DEFINES
    BOARD_USE_TINYUSB
    BOARD_USE_UPDATE_HOOKS
    BOARD_USE_FAST_SOFT_PWM_TIMER
)

add_module(libemueeprom)
add_mcu_library(libemueeprom)

target_compile_definitions(libemueeprom
    PUBLIC
    EMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
)

if (TARGET usb)
    # tinyusb
    target_compile_definitions(usb
        PUBLIC
        CFG_TUD_MSC=0
        CFG_TUD_HID=0
        CFG_TUD_MIDI=1
        CFG_TUD_VENDOR=0
    )
endif()