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
MIDI_BAUD_RATE=31250 \

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
    MIDI_SYSEX_ARRAY_SIZE=45 \
    RING_BUFFER_SIZE=50
endif

#board specific
ifeq ($(findstring opendeck,$(MAKECMDGOALS)), opendeck)
    MCU := atmega32u4
    DEFINES += BOARD_OPEN_DECK
    DEFINES += USB_SUPPORTED
    DEFINES += DIN_MIDI_SUPPORTED
    DEFINES += LED_FADING_SUPPORTED
    DEFINES += LED_INVERT
    DEFINES += USE_MUX
    DEFINES += IN_MATRIX
    DEFINES += OUT_MATRIX
else ifeq ($(findstring leonardo,$(MAKECMDGOALS)), leonardo)
    MCU := atmega32u4
    DEFINES += BOARD_A_LEO
    DEFINES += USB_SUPPORTED
    DEFINES += DIN_MIDI_SUPPORTED
else ifeq ($(findstring pro_micro,$(MAKECMDGOALS)), pro_micro)
    MCU := atmega32u4
    DEFINES += BOARD_A_PRO_MICRO
    DEFINES += USB_SUPPORTED
    DEFINES += DIN_MIDI_SUPPORTED
else ifeq ($(findstring kodama,$(MAKECMDGOALS)), kodama)
    MCU := atmega32u4
    DEFINES += BOARD_KODAMA
    DEFINES += USB_SUPPORTED
    DEFINES += USE_MUX
else ifeq ($(findstring teensy2pp,$(MAKECMDGOALS)), teensy2pp)
    MCU := at90usb1286
    DEFINES += BOARD_T_2PP
    DEFINES += STRING_BUFFER_SIZE=40
    DEFINES += USB_SUPPORTED
    DEFINES += DIN_MIDI_SUPPORTED
    DEFINES += DISPLAY_SUPPORTED
else ifeq ($(findstring mega,$(MAKECMDGOALS)), mega)
    MCU := atmega2560
    DEFINES += BOARD_A_MEGA
    DEFINES += STRING_BUFFER_SIZE=40
    DEFINES += DISPLAY_SUPPORTED
    DEFINES += DIN_MIDI_SUPPORTED
else ifeq ($(findstring uno,$(MAKECMDGOALS)), uno)
    MCU := atmega328p
    DEFINES += BOARD_A_UNO
    DEFINES += DIN_MIDI_SUPPORTED
else ifeq ($(findstring 16u2,$(MAKECMDGOALS)), 16u2)
    MCU := atmega16u2
    DEFINES += BOARD_A_xu2
else ifeq ($(findstring 8u2,$(MAKECMDGOALS)), 8u2)
    MCU := atmega8u2
    DEFINES += BOARD_A_xu2
else ifeq ($(findstring upload,$(MAKECMDGOALS)), upload)
    #hack used to set MCU if only make upload target is called
    #check if MCU file exists
    ifneq ("$(wildcard build/MCU)","")
        MCU := $(shell cat build/MCU)
    else
        $(error Please run make for specific target first)
    endif
    #extra check for uploadboot - fw_opendeck and fw_kodama only for now
    ifeq ($(MAKECMDGOALS),uploadboot)
        ifeq ($(filter fw_opendeck fw_kodama, $(shell cat build/TARGET)), )
            $(error Available only for OpenDeck and Kodama targets.)
        endif
    endif
endif

#mcu specific
ifeq ($(MCU),atmega32u4)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xc8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    EEPROM_SIZE := 1024
    BOOT_START_ADDR := 0x7000
    FLASH_SIZE_START_ADDR := 0xAC
    FLASH_SIZE_END_ADDR := 0xB0
else ifeq ($(MCU),at90usb1286)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    EEPROM_SIZE := 4096
    BOOT_START_ADDR := 0x1F000
    FLASH_SIZE_START_ADDR := 0x98
    FLASH_SIZE_END_ADDR := 0x9C
else ifeq ($(MCU),atmega16u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd3
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    EEPROM_SIZE := 512
    BOOT_START_ADDR := 0x3800
else ifeq ($(MCU),atmega8u2)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd3
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    EEPROM_SIZE := 512
    BOOT_START_ADDR := 0x1800
else ifeq ($(MCU),atmega2560)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd6
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    EEPROM_SIZE := 4096
else ifeq ($(MCU),atmega328p)
    FUSE_UNLOCK := 0xff
    FUSE_EXT := 0x04
    FUSE_HIGH := 0xd6
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    EEPROM_SIZE := 1024
endif

DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
DEFINES += BOOT_START_ADDR=$(BOOT_START_ADDR)
DEFINES += EEPROM_SIZE=$(EEPROM_SIZE)
#for database, total size is three bytes smaller than full eeprom
#one byte for reboot selection and two for crc
DEFINES += LESSDB_SIZE=$(shell echo $(EEPROM_SIZE)-3 | bc)
