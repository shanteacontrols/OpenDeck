vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../
vpath modules/%.c ../

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
common/OpenDeckMIDIformat/OpenDeckMIDIformat.cpp
