ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/lufa/" \
    -I"../modules/avr-libstdcpp/include"

    ifneq (,$(findstring USB_SUPPORTED,$(DEFINES)))
        #common for bootloader and application
        SOURCES += \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c \
        modules/lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.c \
        modules/lufa/LUFA/Drivers/USB/Core/DeviceStandardReq.c \
        modules/lufa/LUFA/Drivers/USB/Core/Events.c \
        modules/lufa/LUFA/Drivers/USB/Core/USBTask.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/CDCClassDevice.c
    endif
endif