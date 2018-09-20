#common defines
DEFINES := \
NDEBUG \
ARCH=ARCH_AVR8 \
F_CPU=16000000UL \
F_USB=16000000UL \
BOARD=BOARD_NONE \
USE_STATIC_OPTIONS=0 \
USB_DEVICE_ONLY \
FIXED_CONTROL_ENDPOINT_SIZE=8 \
FIXED_NUM_CONFIGURATIONS=1 \
HID_VENDOR_ID=0x1209 \
HID_PRODUCT_ID=0x8473 \
MIDI_VENDOR_ID=0x1209 \
MIDI_PRODUCT_ID=0x8472 \
UART_BAUDRATE_MIDI_STD=31250 \
UART_BAUDRATE_MIDI_OD=38400 \
RING_BUFFER_SIZE=50 \
MIDI_SYSEX_ARRAY_SIZE=45

#flash type specific
ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)
    DEFINES += \
    ORDERED_EP_CONFIG \
    NO_SOF_EVENTS \
    USE_RAM_DESCRIPTORS \
    NO_INTERNAL_SERIAL \
    DEVICE_STATE_AS_GPIOR \
    NO_DEVICE_REMOTE_WAKEUP \
    NO_DEVICE_SELF_POWER
else ifeq ($(findstring fw,$(MAKECMDGOALS)), fw)
    DEFINES += \
    USE_FLASH_DESCRIPTORS \
    INTERRUPT_CONTROL_ENDPOINT \
    TOUCHSCREEN_RX_BUFFER_SIZE=20
endif

#board specific
ifeq ($(findstring opendeck,$(MAKECMDGOALS)), opendeck)
    MCU := atmega32u4
    BOARD := BOARD_OPEN_DECK
    DEFINES += LED_FADING_SUPPORTED
    DEFINES += LED_EXT_INVERT
    DEFINES += USE_MUX
    DEFINES += IN_MATRIX
    DEFINES += OUT_MATRIX
    DEFINES += LED_INDICATORS
    DEFINES += CRC_CHECK
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 2
    DEFINES += DIN_MIDI_SUPPORTED
    DEFINES += UART_MIDI_CHANNEL=0
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring tannin,$(MAKECMDGOALS)), tannin)
    MCU := atmega32u4
    BOARD := BOARD_TANNIN
    DEFINES += LED_FADING_SUPPORTED
    DEFINES += LED_EXT_INVERT
    DEFINES += USE_MUX
    DEFINES += IN_MATRIX
    DEFINES += OUT_MATRIX
    DEFINES += CRC_CHECK
    HARDWARE_VERSION_MAJOR := 3
    HARDWARE_VERSION_MINOR := 0
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring leonardo,$(MAKECMDGOALS)), leonardo)
    MCU := atmega32u4
    BOARD := BOARD_A_LEO
    DEFINES += LED_INT_INVERT
    DEFINES += LED_INDICATORS
    DEFINES += CRC_CHECK
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += DIN_MIDI_SUPPORTED
    DEFINES += UART_MIDI_CHANNEL=0
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring pro_micro,$(MAKECMDGOALS)), pro_micro)
    MCU := atmega32u4
    BOARD := BOARD_A_PRO_MICRO
    DEFINES += LED_INT_INVERT
    DEFINES += CRC_CHECK
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += LED_INDICATORS
    DEFINES += DIN_MIDI_SUPPORTED
    DEFINES += UART_MIDI_CHANNEL=0
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring kodama,$(MAKECMDGOALS)), kodama)
    MCU := atmega32u4
    BOARD := BOARD_KODAMA
    DEFINES += USE_MUX
    DEFINES += LED_EXT_INVERT
    DEFINES += CRC_CHECK
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring bergamot,$(MAKECMDGOALS)), bergamot)
    MCU := atmega32u4
    BOARD := BOARD_BERGAMOT
    DEFINES += USE_MUX
    DEFINES += CRC_CHECK
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += TOUCHSCREEN_SUPPORTED
    DEFINES += UART_TOUCHSCREEN_CHANNEL=0
else ifeq ($(findstring teensy2pp,$(MAKECMDGOALS)), teensy2pp)
    MCU := at90usb1286
    BOARD := BOARD_T_2PP
    DEFINES += STRING_BUFFER_SIZE=40
    DEFINES += DISPLAY_SUPPORTED
    DEFINES += CRC_CHECK
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += DIN_MIDI_SUPPORTED
    DEFINES += UART_MIDI_CHANNEL=0
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring mega,$(MAKECMDGOALS)), mega)
    MCU := atmega2560
    BOARD := BOARD_A_MEGA
    DEFINES += STRING_BUFFER_SIZE=40
    DEFINES += DISPLAY_SUPPORTED
    DEFINES += TOUCHSCREEN_SUPPORTED
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += DIN_MIDI_SUPPORTED
    DEFINES += UART_USB_LINK_CHANNEL=0
    DEFINES += UART_MIDI_CHANNEL=1
    DEFINES += UART_TOUCHSCREEN_CHANNEL=1
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring uno,$(MAKECMDGOALS)), uno)
    MCU := atmega328p
    BOARD := BOARD_A_UNO
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += UART_USB_LINK_CHANNEL=0
    DEFINES += LEDS_SUPPORTED
else ifeq ($(findstring 16u2,$(MAKECMDGOALS)), 16u2)
    MCU := atmega16u2
    BOARD := BOARD_A_xu2
    DEFINES += LED_INT_INVERT
    DEFINES += LED_INDICATORS
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += UART_USB_LINK_CHANNEL=0
else ifeq ($(findstring 8u2,$(MAKECMDGOALS)), 8u2)
    MCU := atmega8u2
    BOARD := BOARD_A_xu2
    DEFINES += LED_INT_INVERT
    DEFINES += LED_INDICATORS
    HARDWARE_VERSION_MAJOR := 1
    HARDWARE_VERSION_MINOR := 0
    DEFINES += UART_USB_LINK_CHANNEL=0
else ifeq ($(findstring upload,$(MAKECMDGOALS)), upload)
    #used to set MCU if make upload target is called
    #check if MCU file exists
    ifneq ("$(wildcard build/MCU)","")
        MCU := $(shell cat build/MCU)
    else
        $(error Please run make for specific target first)
    endif
    #only some targets are supported
    ifeq ($(MAKECMDGOALS),uploadboot)
        ifeq ($(filter fw_opendeck fw_tannin fw_leonardo fw_pro_micro fw_kodama fw_teensy2pp fw_bergamot, $(shell cat build/TARGET)), )
            $(error Not available for current target.)
        endif
    endif
endif

#mcu specific
ifeq ($(MCU),atmega32u4)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xc8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 1024
    BOOT_START_ADDR := 0x7000
    FLASH_SIZE_START_ADDR := 0xAC
    FLASH_SIZE_END_ADDR := 0xB0
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),at90usb1286)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 4096
    BOOT_START_ADDR := 0x1F000
    FLASH_SIZE_START_ADDR := 0x98
    FLASH_SIZE_END_ADDR := 0x9C
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),atmega16u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 512
    FLASH_SIZE_START_ADDR := 0x74
    FLASH_SIZE_END_ADDR := 0x78
    BOOT_START_ADDR := 0x3000
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),atmega8u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd3
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 512
    FLASH_SIZE_START_ADDR := 0x74
    FLASH_SIZE_END_ADDR := 0x78
    BOOT_START_ADDR := 0x1800
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),atmega2560)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd4
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 4096
    FLASH_SIZE_START_ADDR := 0xE4
    FLASH_SIZE_END_ADDR := 0xE8
    BOOT_START_ADDR := 0x3F800
    DEFINES += UART_INTERFACES=2
else ifeq ($(MCU),atmega328p)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd6
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 1024
    FLASH_SIZE_START_ADDR := 0x68
    FLASH_SIZE_END_ADDR := 0x6C
    DEFINES += UART_INTERFACES=1
endif

DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
DEFINES += EEPROM_SIZE=$(EEPROM_SIZE)
#for database, total size is four bytes smaller than full eeprom
#one byte for reboot selection
#two for crc
DEFINES += LESSDB_SIZE=$(shell echo $(EEPROM_SIZE)-3 | bc)
DEFINES += $(BOARD)
BOARD_ID := $(shell awk '/$(BOARD)/{print NR-5}' BoardIDs.mk)

ifeq ($(BOARD_ID),)
    $(error Board not added to BoardIDs.mk list)
else
    DEFINES += BOARD_ID=$(BOARD_ID)
endif

DEFINES += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
DEFINES += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)

#append VARIANT to DEFINES only if specified during build
#make sure it's specifed in the following format:
#VARIANT_%
ifneq ($(VARIANT),)
    ifeq ($(findstring VARIANT_,$(VARIANT)), VARIANT_)
        DEFINES += $(VARIANT)
    else
        $(error Wront VARIANT format. Please specify variant in the following manner: "VARIANT=VARIANT_string")
    endif
else ifeq ($(findstring boot_16u2,$(MAKECMDGOALS)), boot_16u2)
    ifeq ($(VARIANT),)
        $(error Variant must be specified for this target)
    endif
endif
