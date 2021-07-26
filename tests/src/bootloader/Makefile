vpath application/%.cpp ../src
vpath bootloader/%.cpp ../src
vpath common/%.cpp ../src

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
bootloader/SysExParser/SysExParser.cpp \
bootloader/updater/Updater.cpp