#common defines

DEFINES := \
UART_BAUDRATE_MIDI_STD=31250 \
UART_BAUDRATE_MIDI_OD=38400 \
FIXED_NUM_CONFIGURATIONS=1

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
else
    DEFINES += NDEBUG
endif

#flash type specific
ifeq ($(findstring boot,$(TARGETNAME)), boot)
    BOARD_DIR := $(subst boot_,,$(TARGETNAME))
else ifeq ($(findstring fw,$(TARGETNAME)), fw)
    BOARD_DIR := $(subst fw_,,$(TARGETNAME))
endif

#determine the architecture by directory in which the board dir is located
ARCH := $(shell find board/ -type d ! -path *build -name *$(BOARD_DIR) | cut -c 7- | cut -d/ -f1 | head -n 1)

#determine MCU by directory in which the board dir is located
MCU := $(shell find board/ -type d -name *$(BOARD_DIR) | cut -c 7- | cut -d/ -f3 | head -n 1)

ifeq ($(TARGETNAME),uploadboot)
    ifeq ($(filter fw_opendeck fw_leonardo fw_promicro fw_dubfocus fw_teensy2pp fw_bergamot fw_mega fw_uno, $(shell cat build/TARGET)), )
        $(error Not available for current target.)
    endif
endif

ifeq ($(MCU), atmega32u4)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xc8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    BOOT_START_ADDR := 0x7000
    FLASH_SIZE_START_ADDR := 0xAC
    FLASH_SIZE_END_ADDR := 0xB0
    DEFINES += __AVR_ATmega32U4__
else ifeq ($(MCU), at90usb1286)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    BOOT_START_ADDR := 0x1F000
    FLASH_SIZE_START_ADDR := 0x98
    FLASH_SIZE_END_ADDR := 0x9C
    DEFINES += __AVR_AT90USB1286__
else ifeq ($(MCU), atmega16u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    FLASH_SIZE_START_ADDR := 0x74
    FLASH_SIZE_END_ADDR := 0x78
    BOOT_START_ADDR := 0x3000
    DEFINES += __AVR_ATmega16U2__
else ifeq ($(MCU), atmega8u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd3
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    FLASH_SIZE_START_ADDR := 0x74
    FLASH_SIZE_END_ADDR := 0x78
    BOOT_START_ADDR := 0x1800
    DEFINES += __AVR_ATmega8U2__
else ifeq ($(MCU), atmega2560)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    FLASH_SIZE_START_ADDR := 0xE4
    FLASH_SIZE_END_ADDR := 0xE8
    BOOT_START_ADDR := 0x3F000
    DEFINES += __AVR_ATmega2560__
else ifeq ($(MCU), atmega328p)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    FLASH_SIZE_START_ADDR := 0x68
    FLASH_SIZE_END_ADDR := 0x6C
    BOOT_START_ADDR := 0x7800
    DEFINES += __AVR_ATmega328P__
else ifeq ($(MCU), stm32f407)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    DEFINES += STM32F407xx
endif

ifeq ($(ARCH),avr)
    #common for all avr targets
    DEFINES += \
    ARCH=ARCH_AVR8 \
    __AVR__ \
    F_CPU=16000000UL \
    F_USB=16000000UL \
    BOARD=BOARD_NONE \
    USE_STATIC_OPTIONS=0 \
    USB_DEVICE_ONLY \
    FIXED_CONTROL_ENDPOINT_SIZE=8

    DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
    DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)

    #flash type specific
    ifeq ($(findstring boot,$(TARGETNAME)), boot)
        DEFINES += \
        ORDERED_EP_CONFIG \
        NO_SOF_EVENTS \
        USE_RAM_DESCRIPTORS \
        DEVICE_STATE_AS_GPIOR \
        NO_DEVICE_REMOTE_WAKEUP \
        NO_DEVICE_SELF_POWER
    else ifeq ($(findstring fw,$(TARGETNAME)), fw)
        DEFINES += \
        USE_FLASH_DESCRIPTORS \
        INTERRUPT_CONTROL_ENDPOINT
    endif
else ifeq ($(ARCH),stm32)
    DEFINES += \
    __STM32__ \
    USE_HAL_DRIVER \
    FIXED_CONTROL_ENDPOINT_SIZE=64 \
    UID_BITS=96
endif

DEFINES += OD_BOARD_$(shell echo $(BOARD_DIR) | tr 'a-z' 'A-Z')
DEFINES += FW_UID=$(shell ../scripts/fw_uid_gen.sh $(TARGETNAME))

ifneq ($(HARDWARE_VERSION_MAJOR), )
    DEFINES += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
endif

ifneq ($(HARDWARE_VERSION_MINOR), )
    DEFINES += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)
endif

ifeq ($(findstring boot,$(TARGETNAME)), boot)
    DEFINES += FW_BOOT
else
    DEFINES += FW_APP
endif

#append VARIANT to DEFINES only if specified during build
#make sure it's specifed in the following format:
#VARIANT_%
ifneq ($(VARIANT),)
    ifeq ($(findstring VARIANT_,$(VARIANT)), VARIANT_)
        DEFINES += $(VARIANT)
    else
        $(error Wront VARIANT format. Please specify variant in the following manner: "VARIANT=VARIANT_string")
    endif
endif
