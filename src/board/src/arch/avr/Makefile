ifeq (,$(findstring gen,$(TYPE)))
    CFLAGS += \
    -fpack-struct

    CPPFLAGS += \
    -flto \
    -mrelax \
    -funsigned-char \
    -funsigned-bitfields \
    -fno-jump-tables

    LDFLAGS += \
    -flto \
    -mrelax
endif

FLASH_TOOL := arduino

DEFINES += \
BOARD_I2C_CUSTOM_IMPL

ifeq ($(TYPE),bootloader)
    DEFINES += \
    NO_SOF_EVENTS \
    DEVICE_STATE_AS_GPIOR \
    NO_DEVICE_REMOTE_WAKEUP \
    NO_DEVICE_SELF_POWER \
    USE_RAM_DESCRIPTORS \
    CORE_USE_C_TIMER_CALLBACK
else ifeq ($(TYPE),application)
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