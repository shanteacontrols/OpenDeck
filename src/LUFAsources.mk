#no lufa for mega or uno
ifeq ($(filter %mega %uno, $(MAKECMDGOALS)), )
    #common for bootloader and firmware
    LUFA_OBJS := \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_R.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_W.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_RW.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Pipe_RW.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.o \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.o \
    modules/lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.o \
    modules/lufa/LUFA/Drivers/USB/Core/DeviceStandardReq.o \
    modules/lufa/LUFA/Drivers/USB/Core/Events.o \
    modules/lufa/LUFA/Drivers/USB/Core/USBTask.o

    #additional sources differ for firmware and bootloader
    ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)
        #bootloader
        LUFA_OBJS += \
        bootloader/hid/BootloaderHID.o \
        bootloader/hid/Descriptors.o \
        modules/lufa/LUFA/Drivers/USB/Class/Common/HIDParser.o \
        modules/lufa/LUFA/Drivers/USB/Class/Device/HIDClassDevice.o
    else
        #firmware
        LUFA_OBJS += \
        firmware/board/avr/usb/Descriptors.o \
        modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.o \
        modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.o
    endif
else
    #no need for lufa
    LUFA_OBJS :=
endif
