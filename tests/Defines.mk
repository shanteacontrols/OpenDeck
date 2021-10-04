#common defines
DEFINES += \
UNITY_INCLUDE_CONFIG_H

TEST_DEFINES := 1
DEFINES += TEST

ifeq ($(HW_TESTING), 1)
    DEFINES += HW_TESTING
endif

#override root path for includes in Makefiles located in src directory
MAKEFILE_INCLUDE_PREFIX := ../src/

include $(MAKEFILE_INCLUDE_PREFIX)Defines.mk
-include $(MAKEFILE_INCLUDE_PREFIX)$(BOARD_TARGET_DIR)/HWTestDefines.mk

ifeq ($(ARCH), stm32)
    DEFINES += STM32_EMU_EEPROM
endif

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out __AVR__,$(DEFINES))
DEFINES := $(filter-out __STM32__,$(DEFINES))
