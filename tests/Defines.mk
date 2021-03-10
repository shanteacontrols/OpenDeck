#common defines
DEFINES += \
UNITY_INCLUDE_CONFIG_H

DEFINES += TEST

include ../src/Defines.mk

#needed for AVR only since this info is normally pulled from AVR headers
#which aren't present in tests
#for stm32 this is determined in runtime
ifeq ($(MCU),at90usb1286)
    DATABASE_SIZE := 4093
else ifeq ($(MCU),atmega16u2)
    DATABASE_SIZE := 509
else ifeq ($(MCU),atmega2560)
    DATABASE_SIZE := 4093
endif

ifeq ($(ARCH), stm32)
    DEFINES += STM32_EMU_EEPROM
endif

DEFINES += OD_BOARD_$(shell echo $(TARGET) | tr 'a-z' 'A-Z')
DEFINES += DATABASE_SIZE=$(DATABASE_SIZE)

DEFINES += SYSEX_MANUFACTURER_ID_0=0x00
DEFINES += SYSEX_MANUFACTURER_ID_1=0x53
DEFINES += SYSEX_MANUFACTURER_ID_2=0x43

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out __AVR__,$(DEFINES))
DEFINES := $(filter-out __STM32__,$(DEFINES))
