vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../
vpath modules/%.c ../

TEST_FRAMEWORK_SOURCES := \
modules/unity/src/unity.c

#common include dirs
INCLUDE_DIRS := \
-I"./" \
-I"../src/application/" \
-I"../src/" \
-I"../modules/" \
-I"../src/board/avr/variants/$(MCU)/$(subst fw_,,$(TARGETNAME))/" \
-isystem "stubs/avr"

INCLUDE_FILES += \
-include "../src/board/avr/variants/$(MCU)/$(subst fw_,,$(TARGETNAME))/Hardware.h" \
-include "../src/board/avr/Config.h"