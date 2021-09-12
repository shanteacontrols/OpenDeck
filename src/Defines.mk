BOARD_GEN_BASE_DIR          := board/gen
BOARD_MCU_BASE_DIR          := $(BOARD_GEN_BASE_DIR)/mcu
BOARD_TARGET_BASE_DIR       := $(BOARD_GEN_BASE_DIR)/target
BOARD_TARGET_DIR            := $(BOARD_TARGET_BASE_DIR)/$(TARGET)
TOUCHSCREEN_GEN_BASE_DIR    := application/io/touchscreen/gen
COMMAND_FW_UPDATE_START     := 0x4F70456E6E45704F
COMMAND_FW_UPDATE_END       := 0x4465436B
SYSEX_MANUFACTURER_ID_0     := 0x00
SYSEX_MANUFACTURER_ID_1     := 0x53
SYSEX_MANUFACTURER_ID_2     := 0x43
FW_METADATA_SIZE            := 4
UART_BAUDRATE_MIDI_STD      := 31250
UART_BAUDRATE_USB           := 38400
USB_OVER_SERIAL_BUFFER_SIZE := 16

SW_VERSION_MAJOR    := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1)
SW_VERSION_MINOR    := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2)
SW_VERSION_REVISION := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3)

DEFINES += \
UART_BAUDRATE_MIDI_STD=$(UART_BAUDRATE_MIDI_STD) \
UART_BAUDRATE_USB=$(UART_BAUDRATE_USB) \
USB_OVER_SERIAL_BUFFER_SIZE=$(USB_OVER_SERIAL_BUFFER_SIZE) \
TSCREEN_CDC_PASSTHROUGH_BUFFER_SIZE=$(USB_OVER_SERIAL_BUFFER_SIZE) \
FIXED_NUM_CONFIGURATIONS=1 \
SYSEX_MANUFACTURER_ID_0=$(SYSEX_MANUFACTURER_ID_0) \
SYSEX_MANUFACTURER_ID_1=$(SYSEX_MANUFACTURER_ID_1) \
SYSEX_MANUFACTURER_ID_2=$(SYSEX_MANUFACTURER_ID_2) \
SW_VERSION_MAJOR=$(SW_VERSION_MAJOR) \
SW_VERSION_MINOR=$(SW_VERSION_MINOR) \
SW_VERSION_REVISION=$(SW_VERSION_REVISION) \
COMMAND_FW_UPDATE_START=$(COMMAND_FW_UPDATE_START) \
COMMAND_FW_UPDATE_END=$(COMMAND_FW_UPDATE_END) \
EMUEEPROM_INCLUDE_CONFIG

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
endif

-include $(MAKEFILE_INCLUDE_PREFIX)$(BOARD_TARGET_DIR)/Defines.mk

ifneq (,$(findstring USB_LINK_MCU,$(DEFINES)))
    ifeq ($(MCU), atmega16u2)
        ifeq ($(TYPE),boot)
            #save flash - this feature doesn't fit into 4k
            DEFINES := $(filter-out LED_INDICATORS_CTL,$(DEFINES))
            DEFINES += UART_TX_BUFFER_SIZE=32
            DEFINES += UART_RX_BUFFER_SIZE=32
        else ifeq ($(TYPE),app)
            DEFINES += UART_TX_BUFFER_SIZE=64
            DEFINES += UART_RX_BUFFER_SIZE=128
        endif
    endif
else
    DEFINES += UART_TX_BUFFER_SIZE=128
    DEFINES += UART_RX_BUFFER_SIZE=128
    DEFINES += MIDI_SYSEX_ARRAY_SIZE=100
endif

ifneq (,$(findstring gen,$(TYPE)))
    #needed only for compilation, unused otherwise for *gen targets
    DEFINES += UID_BITS=96
else
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
        INTERRUPT_CONTROL_ENDPOINT \
        ADC_10_BIT \
        ORDERED_EP_CONFIG \
        UID_BITS=80 \
        MEDIAN_SAMPLE_COUNT=3 \
        MEDIAN_MIDDLE_VALUE=1

        #flash type specific
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
    else ifeq ($(ARCH),stm32)
        DEFINES += \
        __STM32__ \
        USE_HAL_DRIVER \
        UID_BITS=96 \
        USE_USB_FS \
        DEVICE_FS=0 \
        DEVICE_HS=1 \
        ADC_12_BIT
    endif
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
else ifeq ($(TYPE),sysexgen)
    #nothing to do
else
    $(error Invalid firmware type specified)
endif

DEFINES += FLASH_START_ADDR=$(FLASH_START_ADDR)
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
DEFINES += APP_START_ADDR=$(APP_START_ADDR)
DEFINES += FW_METADATA_LOCATION=$(FW_METADATA_LOCATION)

ifneq (,$(findstring UART_CHANNEL,$(DEFINES)))
    DEFINES += USE_UART
endif