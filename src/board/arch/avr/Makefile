ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/core/modules/lufa/" \
    -I"../modules/avr-libstdcpp/include"

    ifneq (,$(findstring USB_SUPPORTED,$(DEFINES)))
        #common for bootloader and application on usb supported mcus
        SOURCES += \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/DeviceStandardReq.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/Events.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Core/USBTask.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c \
        modules/core/modules/lufa/LUFA/Drivers/USB/Class/Device/CDCClassDevice.c
    endif
endif

FLASH_TOOL := arduino

DEFINES += \
CORE_ARCH_AVR \
ARCH=ARCH_AVR8 \
IO_REG_WIDTH=8 \
F_CPU=16000000UL \
F_USB=16000000UL \
BOARD=BOARD_NONE \
USE_STATIC_OPTIONS=0 \
USB_DEVICE_ONLY \
INTERRUPT_CONTROL_ENDPOINT \
ADC_10_BIT \
ORDERED_EP_CONFIG \
USE_CUSTOM_USB_ENDPOINTS

ifeq ($(TYPE),boot)
    DEFINES += \
    NO_SOF_EVENTS \
    DEVICE_STATE_AS_GPIOR \
    NO_DEVICE_REMOTE_WAKEUP \
    NO_DEVICE_SELF_POWER \
    USE_RAM_DESCRIPTORS
else ifeq ($(TYPE),app)
    DEFINES += \
    USE_FLASH_DESCRIPTORS
endif