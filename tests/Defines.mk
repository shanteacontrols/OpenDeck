#common defines
DEFINES += \
TEST \
USE_LOGGER

TEST_DEFINES := 1

#override root path for includes in Makefiles located in src directory
MAKEFILE_INCLUDE_PREFIX := ../src/

include $(MAKEFILE_INCLUDE_PREFIX)Defines.mk
-include $(MAKEFILE_INCLUDE_PREFIX)$(BOARD_GEN_DIR_TARGET)/HWTestDefines.mk

#filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))
DEFINES := $(filter-out INIT_DB_DATA%,$(DEFINES))

DEFINES += INIT_DB_DATA=1