include ../src/Defines.mk

#db size is determined in run time in application firmware, for
#test purposes hardcode it
ifeq ($(MCU),atmega32u4)
    DATABASE_SIZE := 1021
else ifeq ($(MCU),at90usb1286)
    DATABASE_SIZE := 4093
else ifeq ($(MCU),atmega16u2)
    DATABASE_SIZE := 509
else ifeq ($(MCU),atmega8u2)
    DATABASE_SIZE := 509
else ifeq ($(MCU),atmega2560)
    DATABASE_SIZE := 4093
else ifeq ($(MCU),atmega328p)
    DATABASE_SIZE := 1021
else ifeq ($(MCU),stm32f405)
    DATABASE_SIZE := 131068
else ifeq ($(MCU),stm32f407)
    DATABASE_SIZE := 131068
endif

ifeq ($(ARCH), stm32)
    DEFINES += STM32_EMU_EEPROM
endif

DEFINES += OD_BOARD_$(shell echo $(TARGETNAME) | tr 'a-z' 'A-Z')
DEFINES += DATABASE_SIZE=$(DATABASE_SIZE)

DEFINES += SYSEX_MANUFACTURER_ID_0=0x00
DEFINES += SYSEX_MANUFACTURER_ID_1=0x53
DEFINES += SYSEX_MANUFACTURER_ID_2=0x43

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out __AVR__,$(DEFINES))
DEFINES := $(filter-out __STM32__,$(DEFINES))
