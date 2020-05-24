vpath application/%.cpp ../src
vpath common/%.cpp ../src

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
stubs/Core.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp \
application/io/buttons/Buttons.cpp \
application/io/leds/LEDs.cpp \
application/io/common/Common.cpp \
application/io/display/U8X8/U8X8.cpp \
application/io/display/UpdateLogic.cpp \
application/io/display/TextBuild.cpp \
application/io/display/strings/Strings.cpp