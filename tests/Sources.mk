vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../
vpath modules/%.c ../

#common for all targets
COMMON_SOURCES := \
modules/unity/src/unity.c \
modules/dbms/src/LESSDB.cpp \
modules/midi/src/MIDI.cpp \
modules/u8g2/csrc/u8x8_string.c \
modules/u8g2/csrc/u8x8_setup.c \
modules/u8g2/csrc/u8x8_u8toa.c \
modules/u8g2/csrc/u8x8_8x8.c \
modules/u8g2/csrc/u8x8_u16toa.c \
modules/u8g2/csrc/u8x8_display.c \
modules/u8g2/csrc/u8x8_fonts.c \
modules/u8g2/csrc/u8x8_byte.c \
modules/u8g2/csrc/u8x8_cad.c \
modules/u8g2/csrc/u8x8_gpio.c \
modules/u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c \
modules/u8g2/csrc/u8x8_d_ssd1306_128x32.c

#common include dirs
INCLUDE_DIRS_COMMON := \
-I"./" \
-I"../src/application/" \
-I"../src/" \
-I"../modules/" \
-I"../src/board/avr/variants/$(MCU)/$(subst fw_,,$(TARGETNAME))/" \
-isystem "stubs/avr"

INCLUDE_FILES_COMMON += \
-include "../src/board/avr/variants/$(MCU)/$(subst fw_,,$(TARGETNAME))/Hardware.h" \
-include "../src/board/avr/Config.h"