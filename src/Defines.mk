#common defines

COMMAND_FW_UPDATE_START := 0x4F70456E6E45704F
COMMAND_FW_UPDATE_END   := 0x4465436B
SYSEX_MANUFACTURER_ID_0 := 0x00
SYSEX_MANUFACTURER_ID_1 := 0x53
SYSEX_MANUFACTURER_ID_2 := 0x43
FW_METADATA_SIZE        := 4

SW_VERSION_MAJOR    := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1)
SW_VERSION_MINOR    := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2)
SW_VERSION_REVISION := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3)

DEFINES += \
UART_BAUDRATE_MIDI_STD=31250 \
UART_BAUDRATE_MIDI_OD=38400 \
FIXED_NUM_CONFIGURATIONS=1 \
SYSEX_MANUFACTURER_ID_0=$(SYSEX_MANUFACTURER_ID_0) \
SYSEX_MANUFACTURER_ID_1=$(SYSEX_MANUFACTURER_ID_1) \
SYSEX_MANUFACTURER_ID_2=$(SYSEX_MANUFACTURER_ID_2) \
SW_VERSION_MAJOR=$(SW_VERSION_MAJOR) \
SW_VERSION_MINOR=$(SW_VERSION_MINOR) \
SW_VERSION_REVISION=$(SW_VERSION_REVISION)

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
else
    DEFINES += NDEBUG
endif

ARCH := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) arch)
MCU := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) mcu)
MCU_BASE := $(shell echo $(MCU) | rev | cut -c3- | rev)
MCU_FAMILY := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) mcuFamily)

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

    ifeq (,$(findstring TEST,$(DEFINES)))
        FUSE_UNLOCK := $(shell cat board/avr/variants/$(MCU_FAMILY)/$(MCU)/fuses.txt | grep ^unlock= | cut -d= -f2)
        FUSE_EXT := $(shell cat board/avr/variants/$(MCU_FAMILY)/$(MCU)/fuses.txt | grep ^ext= | cut -d= -f2)
        FUSE_HIGH := $(shell cat board/avr/variants/$(MCU_FAMILY)/$(MCU)/fuses.txt | grep ^high= | cut -d= -f2)
        FUSE_LOW := $(shell cat board/avr/variants/$(MCU_FAMILY)/$(MCU)/fuses.txt | grep ^low= | cut -d= -f2)
        FUSE_LOCK := $(shell cat board/avr/variants/$(MCU_FAMILY)/$(MCU)/fuses.txt | grep ^lock= | cut -d= -f2)
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
    ADC_12_BIT \
    HSE_VALUE=$(shell $(YML_PARSER) $(TARGET_DEF_FILE) extClockMhz)000000
endif

FW_UID := $(shell $(SCRIPTS_DIR)/fw_uid_gen.sh $(TARGET))

DEFINES += OD_BOARD=\"$(shell echo $(TARGET))\"
DEFINES += FW_UID=$(FW_UID)
DEFINES += FW_METADATA_LOCATION=$(FW_METADATA_LOCATION)

#set default type to app if left blank
ifeq ($(TYPE),)
    TYPE := app
endif

ifeq ($(TYPE),boot)
    DEFINES += \
    FW_BOOT \
    COMMAND_FW_UPDATE_START=$(COMMAND_FW_UPDATE_START) \
    COMMAND_FW_UPDATE_END=$(COMMAND_FW_UPDATE_END)

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
    DEFINES += \
    COMMAND_FW_UPDATE_START=$(COMMAND_FW_UPDATE_START) \
    COMMAND_FW_UPDATE_END=$(COMMAND_FW_UPDATE_END)
else
    $(error Invalid firmware type specified)
endif

DEFINES += FLASH_START_ADDR=$(FLASH_START_ADDR)
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
DEFINES += APP_START_ADDR=$(APP_START_ADDR)
DEFINES += CDC_START_ADDR=$(CDC_START_ADDR)

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) usb), true)
    DEFINES += USB_MIDI_SUPPORTED
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) usbLink), null)
    ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) usbLink.type), null)
        UART_CHANNEL_USB_LINK := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) usbLink.uartChannel)
        DEFINES += UART_CHANNEL_USB_LINK=$(UART_CHANNEL_USB_LINK)

        ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) usbLink.type), master)
            DEFINES += USB_LINK_MCU
            DEFINES += FW_SELECTOR_NO_VERIFY_CRC

            #append this only if it wasn't appended already
            ifeq (,$(findstring USB_MIDI_SUPPORTED,$(DEFINES)))
                DEFINES += USB_MIDI_SUPPORTED
            endif
        else ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) usbLink.type), slave)
            #make sure slave MCUs don't have USB enabled
            DEFINES := $(filter-out USB_MIDI_SUPPORTED,$(DEFINES))
        endif
    endif
endif

ifneq (,$(findstring USB_LINK_MCU,$(DEFINES)))
#use smaller sysex buffer size on USB link MCUs
    DEFINES += MIDI_SYSEX_ARRAY_SIZE=50
else
    DEFINES += MIDI_SYSEX_ARRAY_SIZE=100
endif

DEFINES += CDC_TX_BUFFER_SIZE=4096
DEFINES += CDC_RX_BUFFER_SIZE=1024

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) dinMIDI), null)
    DEFINES += DIN_MIDI_SUPPORTED
    UART_CHANNEL_DIN=$(shell $(YML_PARSER) $(TARGET_DEF_FILE) dinMIDI.uartChannel)

    ifeq ($(UART_CHANNEL_USB_LINK),$(UART_CHANNEL_DIN))
        $(error USB link channel and DIN MIDI channel cannot be the same)
    endif

    DEFINES += UART_CHANNEL_DIN=$(UART_CHANNEL_DIN)
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) display), null)
    DEFINES += DISPLAY_SUPPORTED
    DEFINES += I2C_CHANNEL_DISPLAY=$(shell $(YML_PARSER) $(TARGET_DEF_FILE) display.i2cChannel)
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) touchscreen), null)
    DEFINES += TOUCHSCREEN_SUPPORTED
    #guard against ommisions of touchscreen component amount by assigning the value to 0 if undefined
    DEFINES += MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS=$(shell $(YML_PARSER) $(TARGET_DEF_FILE) touchscreen.components | grep -v null | awk '{print$$1}END{if(NR==0)print 0}')

ifneq (,$(findstring MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS=0,$(DEFINES)))
    $(error Amount of touchscreen components cannot be 0)
else
    DEFINES += LEDS_SUPPORTED
endif

    UART_CHANNEL_TOUCHSCREEN := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) touchscreen.uartChannel)

    ifeq ($(UART_CHANNEL_USB_LINK),$(UART_CHANNEL_TOUCHSCREEN))
        $(error USB link channel and touchscreen channel cannot be the same)
    endif

    DEFINES += UART_CHANNEL_TOUCHSCREEN=$(UART_CHANNEL_TOUCHSCREEN)
else
    DEFINES += MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS=0
endif

ifeq ($(TYPE),cdc)
    ifeq ($(ARCH), avr)
        $(error CDC not supported for this arch)
    endif
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons), null)
    DEFINES += BUTTONS_SUPPORTED
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.type), native)
    MAX_NUMBER_OF_BUTTONS := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.pins --length)
    DEFINES += NATIVE_BUTTON_INPUTS

    ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.extPullups), true)
        DEFINES += BUTTONS_EXT_PULLUPS
    endif
else ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.type), shiftRegister)
    NUMBER_OF_IN_SR=$(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.shiftRegisters)
    MAX_NUMBER_OF_BUTTONS := $(shell expr 8 \* $(NUMBER_OF_IN_SR))
    DEFINES += NUMBER_OF_IN_SR=$(NUMBER_OF_IN_SR)
else ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.type), matrix)
    ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.columns.pins --length), 3)
        NUMBER_OF_BUTTON_COLUMNS := 8
    else
        $(error Invalid number of columns specified)
    endif
    ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.rows.type), native)
        NUMBER_OF_BUTTON_ROWS := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.rows.pins --length)
    else
        DEFINES += NUMBER_OF_IN_SR=1
        NUMBER_OF_BUTTON_ROWS := 8
    endif
    MAX_NUMBER_OF_BUTTONS := $(shell expr $(NUMBER_OF_BUTTON_COLUMNS) \* $(NUMBER_OF_BUTTON_ROWS))
    DEFINES += NUMBER_OF_BUTTON_COLUMNS=$(NUMBER_OF_BUTTON_COLUMNS)
    DEFINES += NUMBER_OF_BUTTON_ROWS=$(NUMBER_OF_BUTTON_ROWS)
else
    MAX_NUMBER_OF_BUTTONS := 0
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.indexing), null)
    MAX_NUMBER_OF_BUTTONS := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) buttons.indexing --length)
    DEFINES += BUTTON_INDEXING
endif

ifeq ($(shell test $(MAX_NUMBER_OF_BUTTONS) -gt 1; echo $$?),0)
    DEFINES += ENCODERS_SUPPORTED
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog), null)
    DEFINES += ANALOG_SUPPORTED
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.type), native)
    MAX_NUMBER_OF_ANALOG := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.pins --length)
    DEFINES += MAX_ADC_CHANNELS=$(MAX_NUMBER_OF_ANALOG)
    DEFINES += NATIVE_ANALOG_INPUTS
else ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.type), 4067)
    NUMBER_OF_MUX := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.multiplexers)
    DEFINES += NUMBER_OF_MUX=$(NUMBER_OF_MUX)
    DEFINES += NUMBER_OF_MUX_INPUTS=16
    MAX_NUMBER_OF_ANALOG := $(shell expr 16 \* $(NUMBER_OF_MUX))
    DEFINES += MAX_ADC_CHANNELS=$(NUMBER_OF_MUX)
else ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.type), 4051)
    NUMBER_OF_MUX := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.multiplexers)
    DEFINES += NUMBER_OF_MUX=$(NUMBER_OF_MUX)
    DEFINES += NUMBER_OF_MUX_INPUTS=8
    MAX_NUMBER_OF_ANALOG := $(shell expr 8 \* $(NUMBER_OF_MUX))
    DEFINES += MAX_ADC_CHANNELS=$(NUMBER_OF_MUX)
else
    MAX_NUMBER_OF_ANALOG := 0
    MAX_ADC_CHANNELS := 0
    DEFINES += MAX_ADC_CHANNELS=$(MAX_ADC_CHANNELS)
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.indexing), null)
    MAX_NUMBER_OF_ANALOG := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.indexing --length)
    DEFINES += ANALOG_INDEXING
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) analog.extReference), true)
    DEFINES += ADC_EXT_REF
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external), null)
    ifeq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
        DEFINES += LEDS_SUPPORTED
    endif
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.internal.present), true)
    DEFINES += LED_INDICATORS
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.internal.invert), true)
    DEFINES += LED_INT_INVERT
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.invert), true)
    DEFINES += LED_EXT_INVERT
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.fading), true)
    DEFINES += LED_FADING
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) bootloader.button.activeState), high)
    #active high
    DEFINES += BTLDR_BUTTON_AH
endif

ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.type), native)
    MAX_NUMBER_OF_LEDS := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.pins --length)
    DEFINES += NATIVE_LED_OUTPUTS
else ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.type), shiftRegister)
    NUMBER_OF_OUT_SR := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.shiftRegisters)
    MAX_NUMBER_OF_LEDS := $(shell expr 8 \* $(NUMBER_OF_OUT_SR))
    DEFINES += NUMBER_OF_OUT_SR=$(NUMBER_OF_OUT_SR)
else ifeq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.type), matrix)
    NUMBER_OF_LED_COLUMNS := 8
    NUMBER_OF_LED_ROWS := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.rows.pins --length)
    MAX_NUMBER_OF_LEDS := $(shell expr $(NUMBER_OF_LED_COLUMNS) \* $(NUMBER_OF_LED_ROWS))
    DEFINES += NUMBER_OF_LED_COLUMNS=$(NUMBER_OF_LED_COLUMNS)
    DEFINES += NUMBER_OF_LED_ROWS=$(NUMBER_OF_LED_ROWS)
else
    MAX_NUMBER_OF_LEDS := 0
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.indexing), null)
    MAX_NUMBER_OF_LEDS := $(shell $(YML_PARSER) $(TARGET_DEF_FILE) leds.external.indexing --length)
    DEFINES += LED_INDEXING
endif

ifneq ($(shell $(YML_PARSER) $(TARGET_DEF_FILE) unused-io), null)
    DEFINES += TOTAL_UNUSED_IO=$(shell $(YML_PARSER) $(TARGET_DEF_FILE) unused-io --length)
endif

DEFINES += MAX_NUMBER_OF_BUTTONS=$(MAX_NUMBER_OF_BUTTONS)
DEFINES += MAX_NUMBER_OF_ENCODERS=$(shell expr $(MAX_NUMBER_OF_BUTTONS) \/ 2)
DEFINES += MAX_NUMBER_OF_ANALOG=$(MAX_NUMBER_OF_ANALOG)
DEFINES += MAX_NUMBER_OF_LEDS=$(MAX_NUMBER_OF_LEDS)
DEFINES += MAX_NUMBER_OF_RGB_LEDS=$(shell expr $(MAX_NUMBER_OF_LEDS) \/ 3)

ifeq ($(TYPE),cdc)
    DEFINES += USE_UART
else ifneq (,$(findstring UART_CHANNEL,$(DEFINES)))
    DEFINES += USE_UART
endif