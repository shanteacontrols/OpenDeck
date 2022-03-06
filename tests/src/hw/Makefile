vpath application/%.cpp ../src
vpath board/%.cpp ../src

ifeq (,$(findstring USB_LINK_MCU,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    application/database/Database.cpp
endif