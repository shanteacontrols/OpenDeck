#common defines
DEFINES += \
UNITY_INCLUDE_CONFIG_H

TEST_DEFINES := 1
DEFINES += TEST

#override root path for includes in Makefiles located in src directory
MAKEFILE_INCLUDE_PREFIX := ../src/

include $(MAKEFILE_INCLUDE_PREFIX)Defines.mk
-include $(MAKEFILE_INCLUDE_PREFIX)$(GEN_DIR_TARGET)/HWTestDefines.mk

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out __AVR__,$(DEFINES))
DEFINES := $(filter-out __STM32__,$(DEFINES))
DEFINES := $(filter-out __ARM__,$(DEFINES))
