#common defines
DEFINES += \
UNITY_INCLUDE_CONFIG_H

DEFINES += TEST

HW_TEST_FLASH := 1

ifeq ($(HW_TESTING), 1)
    DEFINES += HW_TESTING
endif

ifeq ($(HW_TEST_FLASH), 1)
    DEFINES += HW_TEST_FLASH
endif

-include ../src/board/target/$(TARGET)/Defines.mk
include ../src/Defines.mk

ifeq ($(ARCH), stm32)
    DEFINES += STM32_EMU_EEPROM
endif

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out __AVR__,$(DEFINES))
DEFINES := $(filter-out __STM32__,$(DEFINES))
