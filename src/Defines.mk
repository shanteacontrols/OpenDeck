BOARD_TARGET_BASE_DIR       := board/target
BOARD_TARGET_DIR            := $(BOARD_TARGET_BASE_DIR)/$(TARGET)
TOUCHSCREEN_TARGET_BASE_DIR := application/io/touchscreen/design/target
TOUCHSCREEN_TARGET_DIR      := $(TOUCHSCREEN_TARGET_BASE_DIR)/$(TARGET)
COMMAND_FW_UPDATE_START     := 0x4F70456E6E45704F
COMMAND_FW_UPDATE_END       := 0x4465436B
SYSEX_MANUFACTURER_ID_0     := 0x00
SYSEX_MANUFACTURER_ID_1     := 0x53
SYSEX_MANUFACTURER_ID_2     := 0x43
FW_METADATA_SIZE            := 4
CDC_TX_BUFFER_SIZE          := 4096
CDC_RX_BUFFER_SIZE          := 1024
UART_BAUDRATE_MIDI_STD      := 31250
UART_BAUDRATE_MIDI_OD       := 38400

SW_VERSION_MAJOR    := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1)
SW_VERSION_MINOR    := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2)
SW_VERSION_REVISION := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3)

DEFINES += \
UART_BAUDRATE_MIDI_STD=$(UART_BAUDRATE_MIDI_STD) \
UART_BAUDRATE_MIDI_OD=$(UART_BAUDRATE_MIDI_OD) \
CDC_TX_BUFFER_SIZE=$(CDC_TX_BUFFER_SIZE) \
CDC_RX_BUFFER_SIZE=$(CDC_RX_BUFFER_SIZE) \
FIXED_NUM_CONFIGURATIONS=1 \
SYSEX_MANUFACTURER_ID_0=$(SYSEX_MANUFACTURER_ID_0) \
SYSEX_MANUFACTURER_ID_1=$(SYSEX_MANUFACTURER_ID_1) \
SYSEX_MANUFACTURER_ID_2=$(SYSEX_MANUFACTURER_ID_2) \
SW_VERSION_MAJOR=$(SW_VERSION_MAJOR) \
SW_VERSION_MINOR=$(SW_VERSION_MINOR) \
SW_VERSION_REVISION=$(SW_VERSION_REVISION) \
COMMAND_FW_UPDATE_START=$(COMMAND_FW_UPDATE_START) \
COMMAND_FW_UPDATE_END=$(COMMAND_FW_UPDATE_END)

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
endif

-include $(BOARD_TARGET_DIR)/Defines.mk

ifneq (,$(findstring USB_LINK_MCU,$(DEFINES)))
#use smaller buffer size on USB link MCUs
    DEFINES += UART_TX_BUFFER_SIZE=50
    DEFINES += UART_RX_BUFFER_SIZE=50
    DEFINES += MIDI_SYSEX_ARRAY_SIZE=50
else
    DEFINES += UART_TX_BUFFER_SIZE=200
    DEFINES += UART_RX_BUFFER_SIZE=200
    DEFINES += MIDI_SYSEX_ARRAY_SIZE=100
endif


ARCH := $(shell $(YAML_PARSER) $(TARGET_DEF_FILE) arch)
MCU := $(shell $(YAML_PARSER) $(TARGET_DEF_FILE) mcu)

ifeq ($(MCU), at90usb1286)
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x1E000
    FW_METADATA_LOCATION := 0x98
    DEFINES += __AVR_AT90USB1286__
else ifeq ($(MCU), atmega16u2)
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x3000
    FW_METADATA_LOCATION := 0x74
    DEFINES += __AVR_ATmega16U2__
else ifeq ($(MCU), atmega2560)
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x3E000
    FW_METADATA_LOCATION := 0xE4
    DEFINES += __AVR_ATmega2560__
else ifeq ($(MCU), stm32f407vg)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    CDC_START_ADDR := 0x8020000
    FW_METADATA_LOCATION := $(shell echo $$(($(APP_START_ADDR) + 0x190)))
    DEFINES += STM32F407xx
else ifeq ($(MCU), stm32f405rg)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    CDC_START_ADDR := 0x8020000
    FW_METADATA_LOCATION := $(shell echo $$(($(APP_START_ADDR) + 0x190)))
    DEFINES += STM32F405xx
else ifeq ($(MCU), stm32f401ce)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    CDC_START_ADDR := 0x8020000
    FW_METADATA_LOCATION := $(shell echo $$(($(APP_START_ADDR) + 0x190)))
    DEFINES += STM32F401xE
else ifeq ($(MCU), stm32f411ce)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    CDC_START_ADDR := 0x8020000
    FW_METADATA_LOCATION := $(shell echo $$(($(APP_START_ADDR) + 0x190)))
    DEFINES += STM32F411xE
else
    $(error MCU $(MCU) not supported)
endif

#for *gen applications rewrite the arch to native type
ifneq (,$(findstring gen,$(TYPE)))
    ARCH := native
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
    USE_RAM_DESCRIPTORS \
    ADC_10_BIT

    #flash type specific
    ifeq ($(TYPE),boot)
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
    ADC_12_BIT
endif

ifeq ($(TYPE),boot)
    DEFINES += FW_BOOT
    FLASH_START_ADDR := $(BOOT_START_ADDR)
else ifeq ($(TYPE),app)
    DEFINES += FW_APP
    FLASH_START_ADDR := $(APP_START_ADDR)
else ifeq ($(TYPE),flashgen)
    #same as app
    DEFINES += FW_APP
    FLASH_START_ADDR := $(APP_START_ADDR)
else ifeq ($(TYPE),cdc)
    DEFINES += FW_CDC
    FLASH_START_ADDR := $(CDC_START_ADDR)
else ifeq ($(TYPE),sysexgen)
    #nothing to do
else
    $(error Invalid firmware type specified)
endif

DEFINES += FLASH_START_ADDR=$(FLASH_START_ADDR)
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
DEFINES += APP_START_ADDR=$(APP_START_ADDR)
DEFINES += CDC_START_ADDR=$(CDC_START_ADDR)
DEFINES += FW_METADATA_LOCATION=$(FW_METADATA_LOCATION)

ifeq ($(ARCH), avr)
    ifneq (,$(findstring cdc flashgen,$(TYPE)))
        $(error $(TYPE) not supported for this arch)
    endif
endif

ifeq ($(TYPE),cdc)
    DEFINES += USE_UART
else ifneq (,$(findstring UART_CHANNEL,$(DEFINES)))
    DEFINES += USE_UART
endif