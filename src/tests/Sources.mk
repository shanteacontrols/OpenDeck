vpath application/%.cpp ../
vpath common/%.cpp ../
vpath modules/%.cpp ../
vpath modules/%.c ../

SOURCES_database := \
database/Database.cpp \
modules/dbms/src/LESSDB.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp

SOURCES_buttons := \
interface/digital/input/Buttons.cpp \
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

SOURCES_encoders := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
modules/dbms/src/LESSDB.cpp \
application/interface/digital/input/encoders/Encoders.cpp \
application/interface/digital/input/Common.cpp \
application/database/Database.cpp \
interface/digital/input/Encoders.cpp \
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

SOURCES_pots := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
modules/dbms/src/LESSDB.cpp \
application/interface/analog/Analog.cpp \
application/interface/analog/Potentiometer.cpp \
application/interface/analog/FSR.cpp \
application/interface/digital/output/leds/LEDs.cpp \
application/database/Database.cpp \
interface/analog/Potentiometer.cpp \
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

SOURCES_ringbuf := \
misc/RingBuffer.cpp

SOURCES_odmidi := \
misc/ODMIDIformat.cpp \
common/OpenDeckMIDIformat/OpenDeckMIDIformat.cpp

#common include dirs
INCLUDE_DIRS := \
-I"./" \
-I"../application/" \
-I"../" \
-I"../modules/" \
-I"../application/board/avr/variants/$(subst fw_,,$(TARGETNAME))/" \
-isystem "stubs/avr"

INCLUDE_FILES += \
-include "../application/board/avr/variants/$(subst fw_,,$(TARGETNAME))/Hardware.h" \
-include "stubs/Core.h" \
-include "../application/board/avr/Config.h"