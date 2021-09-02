#common defines
DEFINES += \
UNITY_INCLUDE_CONFIG_H

TEST_DEFINES := 1
DEFINES += TEST

HW_TEST_FLASH := 1

ifeq ($(HW_TESTING), 1)
    DEFINES += HW_TESTING
endif

ifeq ($(HW_TEST_FLASH), 1)
    DEFINES += HW_TEST_FLASH
endif

#redefine this variable since target defines includes MCU defines before anything else
#MCU defines use BOARD_MCU_BASE_DIR as the first line so at that moment it is not defined yet
#ugly but it works
BOARD_MCU_BASE_DIR := ../src/board/gen/mcu
-include ../src/board/gen/target/$(TARGET)/Defines.mk
include ../src/Defines.mk

ifeq ($(ARCH), stm32)
    DEFINES += STM32_EMU_EEPROM
endif

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out __AVR__,$(DEFINES))
DEFINES := $(filter-out __STM32__,$(DEFINES))
