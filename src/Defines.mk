#common defines

COMMAND_FW_UPDATE_START := 0x4F70456E6E45704F
COMMAND_FW_UPDATE_END   := 0x4465436B
SYSEX_MANUFACTURER_ID_0 := 0x00
SYSEX_MANUFACTURER_ID_1 := 0x53
SYSEX_MANUFACTURER_ID_2 := 0x43
FW_METADATA_SIZE        := 4

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

ifeq ($(HAS_BTLDR), 1)
    DEFINES += BOOTLOADER_SUPPORTED
endif

ARCH := $(shell yq r ../targets/$(TARGETNAME).yml arch)
MCU := $(shell yq r ../targets/$(TARGETNAME).yml mcu)
MCU_FAMILY := $(shell yq r ../targets/$(TARGETNAME).yml mcuFamily)

ifeq ($(MCU), at90usb1286)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x1E000
    FW_METADATA_LOCATION := 0x98
    DEFINES += __AVR_AT90USB1286__
else ifeq ($(MCU), atmega16u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x3000
    FW_METADATA_LOCATION := 0x74
    DEFINES += __AVR_ATmega16U2__
else ifeq ($(MCU), atmega2560)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    APP_START_ADDR := 0x00
    BOOT_START_ADDR := 0x3E000
    FW_METADATA_LOCATION := 0xE4
    DEFINES += __AVR_ATmega2560__
else ifeq ($(MCU), stm32f407)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    FW_METADATA_LOCATION := 0x8008190
    DEFINES += STM32F407xx
else ifeq ($(MCU), stm32f405)
    CPU := cortex-m4
    FPU := fpv4-sp-d16
    FLOAT-ABI := hard
    APP_START_ADDR := 0x8008000
    BOOT_START_ADDR := 0x8000000
    FW_METADATA_LOCATION := 0x8008190
    DEFINES += STM32F405xx
else
    $(error MCU $(MCU) not supported)
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
    ADC_12_BIT
else
    $(error Arch $(ARCH) not supported)
endif

FW_UID := $(shell ../scripts/fw_uid_gen.sh $(TARGETNAME))

DEFINES += OD_BOARD_$(shell echo $(TARGETNAME) | tr 'a-z' 'A-Z')
DEFINES += FW_UID=$(FW_UID)
DEFINES += FW_METADATA_LOCATION=$(FW_METADATA_LOCATION)

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
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
DEFINES += APP_START_ADDR=$(APP_START_ADDR)

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml usb), true)
    DEFINES += USB_MIDI_SUPPORTED
endif

ifneq ($(shell yq r ../targets/$(TARGETNAME).yml usbLink),)
    ifneq ($(shell yq r ../targets/$(TARGETNAME).yml usbLink.type),)
        UART_CHANNEL_USB_LINK := $(shell yq r ../targets/$(TARGETNAME).yml usbLink.uartChannel)
        DEFINES += UART_CHANNEL_USB_LINK=$(UART_CHANNEL_USB_LINK)

        ifeq ($(shell yq r ../targets/$(TARGETNAME).yml usbLink.type), master)
            DEFINES += USB_LINK_MCU

            #append this only if it wasn't appended already
            ifeq (,$(findstring USB_MIDI_SUPPORTED,$(DEFINES)))
                DEFINES += USB_MIDI_SUPPORTED
            endif
        else ifeq ($(shell yq r ../targets/$(TARGETNAME).yml usbLink.type), slave)
            #make sure slave MCUs don't have USB enabled
            DEFINES := $(filter-out USB_MIDI_SUPPORTED,$(DEFINES))
        endif
    endif
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml dinMIDI.use), true)
    DEFINES += DIN_MIDI_SUPPORTED
    UART_CHANNEL_DIN=$(shell yq r ../targets/$(TARGETNAME).yml dinMIDI.uartChannel)

    ifeq ($(UART_CHANNEL_USB_LINK),$(UART_CHANNEL_DIN))
        $(error USB link channel and DIN MIDI channel cannot be the same)
    endif

    DEFINES += UART_CHANNEL_DIN=$(UART_CHANNEL_DIN)
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml display.use), true)
    DEFINES += DISPLAY_SUPPORTED
    DEFINES += I2C_CHANNEL_DISPLAY=$(shell yq r ../targets/$(TARGETNAME).yml display.i2cChannel)
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml touchscreen.use), true)
    DEFINES += TOUCHSCREEN_SUPPORTED
    DEFINES += MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS=$(shell yq r ../targets/$(TARGETNAME).yml touchscreen.components)
    DEFINES += LEDS_SUPPORTED

    UART_CHANNEL_TOUCHSCREEN := $(shell yq r ../targets/$(TARGETNAME).yml touchscreen.uartChannel)

    ifeq ($(UART_CHANNEL_USB_LINK),$(UART_CHANNEL_TOUCHSCREEN))
        $(error USB link channel and touchscreen channel cannot be the same)
    endif

    DEFINES += UART_CHANNEL_TOUCHSCREEN=$(UART_CHANNEL_TOUCHSCREEN)
else
    DEFINES += MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS=0
endif

ifneq ($(shell yq r ../targets/$(TARGETNAME).yml buttons),)
    DEFINES += BUTTONS_SUPPORTED
    DEFINES += ENCODERS_SUPPORTED
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml buttons.type), native)
    MAX_NUMBER_OF_BUTTONS := $(shell yq r ../targets/$(TARGETNAME).yml buttons.pins --length)
    DEFINES += NATIVE_BUTTON_INPUTS

    ifeq ($(shell yq r ../targets/$(TARGETNAME).yml buttons.extPullups), true)
        DEFINES += BUTTONS_EXT_PULLUPS
    endif
else ifeq ($(shell yq r ../targets/$(TARGETNAME).yml buttons.type), shiftRegister)
    NUMBER_OF_IN_SR=$(shell yq r ../targets/$(TARGETNAME).yml buttons.shiftRegisters)
    MAX_NUMBER_OF_BUTTONS := $(shell expr 8 \* $(NUMBER_OF_IN_SR))
    DEFINES += NUMBER_OF_IN_SR=$(NUMBER_OF_IN_SR)
else ifeq ($(shell yq r ../targets/$(TARGETNAME).yml buttons.type), matrix)
    NUMBER_OF_BUTTON_COLUMNS := $(shell yq r ../targets/$(TARGETNAME).yml buttons.columns)
    NUMBER_OF_BUTTON_ROWS := $(shell yq r ../targets/$(TARGETNAME).yml buttons.rows)
    MAX_NUMBER_OF_BUTTONS := $(shell expr $(NUMBER_OF_BUTTON_COLUMNS) \* $(NUMBER_OF_BUTTON_ROWS))
    DEFINES += NUMBER_OF_BUTTON_COLUMNS=$(NUMBER_OF_BUTTON_COLUMNS)
    DEFINES += NUMBER_OF_BUTTON_ROWS=$(NUMBER_OF_BUTTON_ROWS)
    DEFINES += NUMBER_OF_IN_SR=1
else
    MAX_NUMBER_OF_BUTTONS := 0
endif

ifneq ($(shell yq r ../targets/$(TARGETNAME).yml buttons.indexing),)
    MAX_NUMBER_OF_BUTTONS := $(shell yq r ../targets/$(TARGETNAME).yml buttons.indexing --length)
    DEFINES += BUTTON_INDEXING
endif

ifneq ($(shell yq r ../targets/$(TARGETNAME).yml analog),)
    DEFINES += ANALOG_SUPPORTED
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml analog.type), native)
    MAX_NUMBER_OF_ANALOG := $(shell yq r ../targets/$(TARGETNAME).yml analog.pins --length)
    DEFINES += MAX_ADC_CHANNELS=$(MAX_NUMBER_OF_ANALOG)
    DEFINES += NATIVE_ANALOG_INPUTS
else ifeq ($(shell yq r ../targets/$(TARGETNAME).yml analog.type), 4067)
    NUMBER_OF_MUX := $(shell yq r ../targets/$(TARGETNAME).yml analog.multiplexers)
    DEFINES += NUMBER_OF_MUX=$(NUMBER_OF_MUX)
    DEFINES += NUMBER_OF_MUX_INPUTS=16
    MAX_NUMBER_OF_ANALOG := $(shell expr 16 \* $(NUMBER_OF_MUX))
    DEFINES += MAX_ADC_CHANNELS=$(NUMBER_OF_MUX)
else ifeq ($(shell yq r ../targets/$(TARGETNAME).yml analog.type), 4051)
    NUMBER_OF_MUX := $(shell yq r ../targets/$(TARGETNAME).yml analog.multiplexers)
    DEFINES += NUMBER_OF_MUX=$(NUMBER_OF_MUX)
    DEFINES += NUMBER_OF_MUX_INPUTS=8
    MAX_NUMBER_OF_ANALOG := $(shell expr 8 \* $(NUMBER_OF_MUX))
    DEFINES += MAX_ADC_CHANNELS=$(NUMBER_OF_MUX)
else
    MAX_NUMBER_OF_ANALOG := 0
    MAX_ADC_CHANNELS := 0
    DEFINES += MAX_ADC_CHANNELS=$(MAX_ADC_CHANNELS)
endif

ifneq ($(shell yq r ../targets/$(TARGETNAME).yml analog.indexing),)
    MAX_NUMBER_OF_ANALOG := $(shell yq r ../targets/$(TARGETNAME).yml analog.indexing --length)
    DEFINES += ANALOG_INDEXING
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml analog.extReference), true)
    DEFINES += ADC_EXT_REF
endif

ifneq ($(shell yq r ../targets/$(TARGETNAME).yml leds.external),)
    ifeq (,$(findstring LEDS_SUPPORTED,$(DEFINES)))
        DEFINES += LEDS_SUPPORTED
    endif
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml leds.internal.present), true)
    DEFINES += LED_INDICATORS
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml leds.internal.invert), true)
    DEFINES += LED_INT_INVERT
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml leds.external.invert), true)
    DEFINES += LED_EXT_INVERT
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml leds.external.fading), true)
    DEFINES += LED_FADING
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml bootloader.button.activeState), true)
    #active high
    DEFINES += BTLDR_BUTTON_AH
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml leds.external.type), native)
    MAX_NUMBER_OF_LEDS := $(shell yq r ../targets/$(TARGETNAME).yml leds.external.pins --length)
    DEFINES += NATIVE_LED_OUTPUTS
else ifeq ($(shell yq r ../targets/$(TARGETNAME).yml leds.external.type), shiftRegister)
    NUMBER_OF_OUT_SR := $(shell yq r ../targets/$(TARGETNAME).yml leds.external.shiftRegisters)
    MAX_NUMBER_OF_LEDS := $(shell expr 8 \* $(NUMBER_OF_OUT_SR))
    DEFINES += NUMBER_OF_OUT_SR=$(NUMBER_OF_OUT_SR)
else ifeq ($(shell yq r ../targets/$(TARGETNAME).yml leds.external.type), matrix)
    NUMBER_OF_LED_COLUMNS := $(shell yq r ../targets/$(TARGETNAME).yml leds.external.columns)
    NUMBER_OF_LED_ROWS := $(shell yq r ../targets/$(TARGETNAME).yml leds.external.rows)
    MAX_NUMBER_OF_LEDS := $(shell expr $(NUMBER_OF_LED_COLUMNS) \* $(NUMBER_OF_LED_ROWS))
    DEFINES += NUMBER_OF_LED_COLUMNS=$(NUMBER_OF_LED_COLUMNS)
    DEFINES += NUMBER_OF_LED_ROWS=$(NUMBER_OF_LED_ROWS)
else
    MAX_NUMBER_OF_LEDS := 0
endif

ifneq ($(shell yq r ../targets/$(TARGETNAME).yml leds.external.indexing),)
    MAX_NUMBER_OF_LEDS := $(shell yq r ../targets/$(TARGETNAME).yml leds.external.indexing --length)
    DEFINES += LED_INDEXING
endif

DEFINES += MAX_NUMBER_OF_BUTTONS=$(MAX_NUMBER_OF_BUTTONS)
DEFINES += MAX_NUMBER_OF_ENCODERS=$(shell expr $(MAX_NUMBER_OF_BUTTONS) \/ 2)
DEFINES += MAX_NUMBER_OF_ANALOG=$(MAX_NUMBER_OF_ANALOG)
DEFINES += MAX_NUMBER_OF_LEDS=$(MAX_NUMBER_OF_LEDS)
DEFINES += MAX_NUMBER_OF_RGB_LEDS=$(shell expr $(MAX_NUMBER_OF_LEDS) \/ 3)

ifneq (,$(findstring UART_CHANNEL,$(DEFINES)))
    DEFINES += USE_UART
endif