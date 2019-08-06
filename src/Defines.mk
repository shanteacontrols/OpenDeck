#common defines

DEFINES := \
NDEBUG \
HID_VENDOR_ID=0x1209 \
HID_PRODUCT_ID=0x8473 \
MIDI_VENDOR_ID=0x1209 \
MIDI_PRODUCT_ID=0x8472 \
UART_BAUDRATE_MIDI_STD=31250 \
UART_BAUDRATE_MIDI_OD=38400

#flash type specific
ifeq ($(findstring boot,$(TARGETNAME)), boot)
    BOARD_DIR := $(subst boot_,,$(TARGETNAME))
else ifeq ($(findstring fw,$(TARGETNAME)), fw)
    BOARD_DIR := $(subst fw_,,$(TARGETNAME))
endif

#board specific
ifeq ($(BOARD_DIR), opendeck)
    MCU := atmega32u4
    BOARD := BOARD_OPEN_DECK
else ifeq ($(BOARD_DIR), leonardo)
    MCU := atmega32u4
    BOARD := BOARD_A_LEO
else ifeq ($(BOARD_DIR), pro_micro)
    MCU := atmega32u4
    BOARD := BOARD_A_PRO_MICRO
else ifeq ($(BOARD_DIR), dubfocus)
    MCU := atmega32u4
    BOARD := BOARD_DUBFOCUS
else ifeq ($(BOARD_DIR), bergamot)
    MCU := atmega32u4
    BOARD := BOARD_BERGAMOT
else ifeq ($(BOARD_DIR), teensy2pp)
    MCU := at90usb1286
    BOARD := BOARD_T_2PP
else ifeq ($(BOARD_DIR), mega)
    MCU := atmega2560
    BOARD := BOARD_A_MEGA
else ifeq ($(BOARD_DIR), mega6mux)
    MCU := atmega2560
    BOARD := BOARD_A_MEGA6MUX
else ifeq ($(BOARD_DIR), uno)
    MCU := atmega328p
    BOARD := BOARD_A_UNO
else ifeq ($(BOARD_DIR), 16u2)
    MCU := atmega16u2
    BOARD := BOARD_A_xu2
else ifeq ($(BOARD_DIR), 8u2)
    MCU := atmega8u2
    BOARD := BOARD_A_xu2
endif

ifeq ($(findstring upload,$(MAKECMDGOALS)), upload)
    #used to set MCU if make upload target is called
    #check if MCU file exists
    ifneq ("$(wildcard build/MCU)","")
        MCU := $(shell cat build/MCU)
    else
        $(error Please run make for specific target first)
    endif
    #only some targets are supported
    ifeq ($(TARGETNAME),uploadboot)
        ifeq ($(filter fw_opendeck fw_leonardo fw_pro_micro fw_dubfocus fw_teensy2pp fw_bergamot fw_mega fw_uno, $(shell cat build/TARGET)), )
            $(error Not available for current target.)
        endif
    endif
endif

#mcu specific
ifeq ($(MCU),atmega32u4)
    ARCH := avr
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
    DEFINES += __AVR_ATmega32U4__
else ifeq ($(MCU),at90usb1286)
    ARCH := avr
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
    DEFINES += __AVR_AT90USB1286__
else ifeq ($(MCU),atmega16u2)
    ARCH := avr
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
    DEFINES += __AVR_ATmega16U2__
else ifeq ($(MCU),atmega8u2)
    ARCH := avr
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
    DEFINES += __AVR_ATmega8U2__
else ifeq ($(MCU),atmega2560)
    ARCH := avr
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 4096
    FLASH_SIZE_START_ADDR := 0xE4
    FLASH_SIZE_END_ADDR := 0xE8
    BOOT_START_ADDR := 0x3F000
    DEFINES += UART_INTERFACES=2
    DEFINES += __AVR_ATmega2560__
else ifeq ($(MCU),atmega328p)
    ARCH := avr
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0xef
    EEPROM_SIZE := 1024
    FLASH_SIZE_START_ADDR := 0x68
    FLASH_SIZE_END_ADDR := 0x6C
    BOOT_START_ADDR := 0x7800
    DEFINES += UART_INTERFACES=1
    DEFINES += __AVR_ATmega328P__
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
    FIXED_NUM_CONFIGURATIONS=1

    DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
    DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
    DEFINES += EEPROM_SIZE=$(EEPROM_SIZE)

    #flash type specific
    ifeq ($(findstring boot,$(TARGETNAME)), boot)
        DEFINES += \
        ORDERED_EP_CONFIG \
        NO_SOF_EVENTS \
        USE_RAM_DESCRIPTORS \
        NO_INTERNAL_SERIAL \
        DEVICE_STATE_AS_GPIOR \
        NO_DEVICE_REMOTE_WAKEUP \
        NO_DEVICE_SELF_POWER
    else ifeq ($(findstring fw,$(TARGETNAME)), fw)
        DEFINES += \
        USE_FLASH_DESCRIPTORS \
        INTERRUPT_CONTROL_ENDPOINT
    endif
endif

DEFINES += $(BOARD)
DEFINES += FW_UID=$(shell ../scripts/fw_uid_gen.sh $(TARGETNAME))

ifneq ($(HARDWARE_VERSION_MAJOR), )
    DEFINES += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
endif

ifneq ($(HARDWARE_VERSION_MINOR), )
    DEFINES += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)
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
else ifeq ($(findstring boot_16u2,$(TARGETNAME)), boot_16u2)
    ifeq ($(VARIANT),)
        $(error Variant must be specified for this target)
    endif
endif
