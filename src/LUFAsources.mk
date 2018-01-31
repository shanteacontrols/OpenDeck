#common lufa objects

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

ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)

LUFA_OBJS += \
bootloader/hid/BootloaderHID.o \
bootloader/hid/Descriptors.o \
modules/lufa/LUFA/Drivers/USB/Class/Common/HIDParser.o \
modules/lufa/LUFA/Drivers/USB/Class/Device/HIDClassDevice.o

endif

ifeq ($(findstring fw,$(MAKECMDGOALS)), fw)

#no lufa for mega
ifneq ($(MAKECMDGOALS),fw_mega)
LUFA_OBJS += \
firmware/board/avr/usb/Descriptors.o \
modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.o \
modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.o
else
LUFA_OBJS :=
endif

endif