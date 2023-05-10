vpath application/%.cpp ../src/firmware

ifeq (,$(findstring PROJECT_TARGET_USB_OVER_SERIAL_HOST,$(DEFINES)))
    SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
    application/database/Database.cpp \
    application/database/CustomInit.cpp
endif