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
STRING_BUFFER_SIZE=1 \
MIDI_BAUD_RATE=31250

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
    RING_BUFFER_SIZE=50 \
    DBMS_MAX_SECTIONS=7 \
    DBMS_MAX_BLOCKS=7 \
    SYSEX_MAX_BLOCKS=7 \
    SYSEX_MAX_SECTIONS=10
endif

#board specific
ifeq ($(findstring opendeck,$(MAKECMDGOALS)), opendeck)
    MCU := atmega32u4
    DEFINES += BOARD_OPEN_DECK
else ifeq ($(findstring leonardo,$(MAKECMDGOALS)), leonardo)
    MCU := atmega32u4
    DEFINES += BOARD_A_LEO
else ifeq ($(findstring pro_micro,$(MAKECMDGOALS)), pro_micro)
    MCU := atmega32u4
    DEFINES += BOARD_A_PRO_MICRO
else ifeq ($(findstring teensy2pp,$(MAKECMDGOALS)), teensy2pp)
    MCU := at90usb1286
    DEFINES += BOARD_T_2PP
else ifeq ($(findstring mega,$(MAKECMDGOALS)), mega)
    MCU := atmega2560
    DEFINES += BOARD_A_MEGA
else ifeq ($(findstring uno,$(MAKECMDGOALS)), uno)
    MCU := atmega328p
    DEFINES += BOARD_A_UNO
else ifeq ($(findstring 16u2,$(MAKECMDGOALS)), 16u2)
    MCU := atmega16u2
    DEFINES += BOARD_A_16u2
endif

#mcu specific
ifeq ($(MCU),atmega32u4)
    FUSE_UNLOCK := 0x3f
    FUSE_EXT := 0xc8
    FUSE_HIGH := 0xd0
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    DEFINES += EEPROM_SIZE=1024
    BOOT_START_ADDR := 0x7000
    FLASH_SIZE_START_ADDR := 0xAC
    FLASH_SIZE_END_ADDR := 0xB0
    MCU_avrdude := m32u4
else ifeq ($(MCU),at90usb1286)
    FUSE_UNLOCK := 0x3f
    FUSE_EXT := 0xf8
    FUSE_HIGH := 0xd2
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    DEFINES += EEPROM_SIZE=4096
    BOOT_START_ADDR := 0x1F000
    FLASH_SIZE_START_ADDR := 0x98
    FLASH_SIZE_END_ADDR := 0x9C
    MCU_avrdude := usb1286
else ifeq ($(MCU),atmega16u2)
    FUSE_UNLOCK := 0x3f
    FUSE_EXT := 0xf0
    FUSE_HIGH := 0xd3
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    DEFINES += EEPROM_SIZE=512
    BOOT_START_ADDR := 0x3000
    MCU_avrdude := m16u2
else ifeq ($(MCU),atmega2560)
    FUSE_UNLOCK := 0x3f
    FUSE_EXT := 0xfc
    FUSE_HIGH := 0xd6
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    DEFINES += EEPROM_SIZE=4096
    MCU_avrdude := m2560
else ifeq ($(MCU),atmega328p)
    FUSE_UNLOCK := 0x3f
    FUSE_EXT := 0x04
    FUSE_HIGH := 0xd6
    FUSE_LOW := 0xff
    FUSE_LOCK := 0x2f
    DEFINES += EEPROM_SIZE=1024
    MCU_avrdude := m328p
endif

DEFINES += BOOT_START_ADDR
