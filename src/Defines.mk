#common defines

COMMAND_FW_UPDATE_START := 0x4F70656E
COMMAND_FW_UPDATE_END   := 0x4465636B
SYSEX_MANUFACTURER_ID_0 := 0x00
SYSEX_MANUFACTURER_ID_1 := 0x53
SYSEX_MANUFACTURER_ID_2 := 0x43

DEFINES := \
UART_BAUDRATE_MIDI_STD=31250 \
UART_BAUDRATE_MIDI_OD=38400 \
FIXED_NUM_CONFIGURATIONS=1 \
SYSEX_MANUFACTURER_ID_0=$(SYSEX_MANUFACTURER_ID_0) \
SYSEX_MANUFACTURER_ID_1=$(SYSEX_MANUFACTURER_ID_1) \
SYSEX_MANUFACTURER_ID_2=$(SYSEX_MANUFACTURER_ID_2)

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
else
    DEFINES += NDEBUG
endif

BOARD_DIR := $(TARGETNAME)

#determine the architecture, mcu and mcu family by directory in which the board dir is located
ARCH := $(shell $(FIND) board -type d ! -path *build -name *$(BOARD_DIR) | cut -d/ -f2 | head -n 1)
MCU := $(shell $(FIND) board -type d -name *$(BOARD_DIR) | cut -d/ -f5 | head -n 1)
MCU_FAMILY := $(shell $(FIND) board -type d -name *$(BOARD_DIR) | cut -d/ -f4 | head -n 1)

ifeq ($(MCU), atmega32u4)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xc8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x7000
    DEFINES += __AVR_ATmega32U4__
else ifeq ($(MCU), at90usb1286)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x1E000
    DEFINES += __AVR_AT90USB1286__
else ifeq ($(MCU), atmega16u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x3000
    DEFINES += __AVR_ATmega16U2__
else ifeq ($(MCU), atmega8u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd3
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x1800
    DEFINES += __AVR_ATmega8U2__
else ifeq ($(MCU), atmega2560)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x3E000
    DEFINES += __AVR_ATmega2560__
else ifeq ($(MCU), atmega328p)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x7000
    DEFINES += __AVR_ATmega328P__
else ifeq ($(MCU), stm32f407)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    DEFINES += STM32F407xx
else ifeq ($(MCU), stm32f405)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    DEFINES += STM32F405xx
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
    FIXED_CONTROL_ENDPOINT_SIZE=8 \
    INTERRUPT_CONTROL_ENDPOINT \
    USE_RAM_DESCRIPTORS

    DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
    DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)

    #flash type specific
    ifeq ($(BOOT),1)
        DEFINES += \
        ORDERED_EP_CONFIG \
        NO_SOF_EVENTS \
        DEVICE_STATE_AS_GPIOR \
        NO_DEVICE_REMOTE_WAKEUP \
        NO_DEVICE_SELF_POWER
    endif
else ifeq ($(ARCH),stm32)
    DEFINES += \
    __STM32__ \
    USE_HAL_DRIVER \
    FIXED_CONTROL_ENDPOINT_SIZE=64 \
    UID_BITS=96 \
    USE_USB_FS \
    DEVICE_FS=0 \
    DEVICE_HS=1 \
    EEPROM_RAM_CACHE
endif

DEFINES += OD_BOARD_$(shell echo $(BOARD_DIR) | tr 'a-z' 'A-Z')
DEFINES += FW_UID=$(shell ../scripts/fw_uid_gen.sh $(TARGETNAME) $(BOOT))

ifneq ($(HARDWARE_VERSION_MAJOR), )
    DEFINES += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
endif

ifneq ($(HARDWARE_VERSION_MINOR), )
    DEFINES += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)
endif

ifeq ($(BOOT),1)
    DEFINES += \
    FW_BOOT \
    COMMAND_FW_UPDATE_START=$(COMMAND_FW_UPDATE_START) \
    COMMAND_FW_UPDATE_END=$(COMMAND_FW_UPDATE_END)

    FLASH_START_ADDR := $(BOOT_START_ADDR)
else
    DEFINES += FW_APP
    FLASH_START_ADDR := $(APP_START_ADDR)
endif

DEFINES += FLASH_START_ADDR=$(FLASH_START_ADDR)
DEFINES += APP_START_ADDR=$(APP_START_ADDR)
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
