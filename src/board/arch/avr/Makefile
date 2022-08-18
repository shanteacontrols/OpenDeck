ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/core/modules/lufa/" \
    -I"../modules/avr-libstdcpp/include"

    ifneq (,$(findstring HW_SUPPORT_USB,$(DEFINES)))
        # Common for bootloader and application on USB supported MCUs
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

    CFLAGS += \
    -fpack-struct

    CPPFLAGS += \
    -mrelax \
    -mmcu=$(CORE_MCU_MODEL) \
    -funsigned-char \
    -funsigned-bitfields \
    -fno-jump-tables

    LDFLAGS += \
    -Wl,--section-start=.text=$(FLASH_START_ADDR) \
    -mrelax \
    -mmcu=$(CORE_MCU_MODEL)

    ifeq ($(DEBUG), 1)
        CPPFLAGS += -g -gdwarf-2
        LDFLAGS  += -g -gdwarf-2
        OPT      := -Og
    endif
endif

FLASH_TOOL := arduino

DEFINES += \
BOARD=BOARD_NONE \
USE_STATIC_OPTIONS=0 \
USB_DEVICE_ONLY \
INTERRUPT_CONTROL_ENDPOINT \
ORDERED_EP_CONFIG \
USE_CUSTOM_USB_ENDPOINTS \
BOARD_I2C_CUSTOM_IMPL

ifeq ($(TYPE),boot)
    DEFINES += \
    NO_SOF_EVENTS \
    DEVICE_STATE_AS_GPIOR \
    NO_DEVICE_REMOTE_WAKEUP \
    NO_DEVICE_SELF_POWER \
    USE_RAM_DESCRIPTORS \
    CORE_USE_C_TIMER_CALLBACK
else ifeq ($(TYPE),app)
    DEFINES += \
    USE_FLASH_DESCRIPTORS
endif

CC             := avr-gcc
CXX            := avr-g++
LD             := avr-g++
AS             := $(CC)
SIZE_TOOL_BASE := avr-size
SIZE_TOOL      := $(SIZE_TOOL_BASE) -C --mcu=$(CORE_MCU_MODEL)

REQ_PACKAGES := \
$(CC) \
$(CXX) \
$(LD) \
$(AS) \
$(SIZE_TOOL_BASE)

$(call CHECK_PACKAGES,$(REQ_PACKAGES))

flashread:
	$(Q)avrdude -p $(CORE_MCU_MODEL) -P /dev/$(PORT) -b 19200 -c avrisp -C /etc/avrdude.conf -U flash:r:readcontents.bin:r