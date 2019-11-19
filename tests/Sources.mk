vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../src
vpath modules/%.c ../src

TEST_FRAMEWORK_SOURCES := \
modules/unity/src/unity.c

SOURCES_test_Database := \
modules/dbms/src/LESSDB.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp

SOURCES_test_Buttons := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
modules/dbms/src/LESSDB.cpp \
application/interface/digital/input/buttons/Buttons.cpp \
application/interface/digital/input/buttons/Hooks.cpp \
application/interface/digital/output/leds/LEDs.cpp \
application/interface/digital/input/Common.cpp \
application/database/Database.cpp \
modules/midi/src/MIDI.cpp \
application/interface/display/U8X8/U8X8.cpp \
application/interface/display/UpdateLogic.cpp \
application/interface/display/TextBuild.cpp \
application/interface/display/strings/Strings.cpp \
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

SOURCES_test_Encoders := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
modules/dbms/src/LESSDB.cpp \
application/interface/digital/input/encoders/Encoders.cpp \
application/interface/digital/input/Common.cpp \
application/database/Database.cpp \
modules/midi/src/MIDI.cpp \
application/interface/display/U8X8/U8X8.cpp \
application/interface/display/UpdateLogic.cpp \
application/interface/display/TextBuild.cpp \
application/interface/display/strings/Strings.cpp \
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

SOURCES_test_Potentiometer := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
modules/dbms/src/LESSDB.cpp \
application/interface/analog/Analog.cpp \
application/interface/analog/Potentiometer.cpp \
application/interface/analog/FSR.cpp \
application/interface/digital/output/leds/LEDs.cpp \
application/database/Database.cpp \
modules/midi/src/MIDI.cpp \
application/interface/display/U8X8/U8X8.cpp \
application/interface/display/UpdateLogic.cpp \
application/interface/display/TextBuild.cpp \
application/interface/display/strings/Strings.cpp \
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

SOURCES_test_RingBuffer := \

SOURCES_test_ODMIDIformat := \
common/OpenDeckMIDIformat/OpenDeckMIDIformat.cpp

#common include dirs
INCLUDE_DIRS := \
-I"./" \
-I"../src/application/" \
-I"../src/" \
-I"../src/modules/" \
-I"../src/board/avr/variants/$(MCU)/$(subst fw_,,$(TARGETNAME))/" \
-isystem "stubs/avr"

INCLUDE_FILES += \
-include "../src/board/avr/variants/$(MCU)/$(subst fw_,,$(TARGETNAME))/Hardware.h" \
-include "../src/board/avr/Config.h"