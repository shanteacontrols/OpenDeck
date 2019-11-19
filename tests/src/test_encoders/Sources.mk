vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../
vpath modules/%.c ../

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
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