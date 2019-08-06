#slightly modified Defines.mk from src directory

#common
DEFINES := \
F_CPU=16000000UL

TARGETNAME := fw_opendeck

#board specific
ifeq ($(TARGETNAME), fw_opendeck)
    MCU := atmega32u4
    BOARD := BOARD_OPEN_DECK
else ifeq ($(TARGETNAME), fw_leonardo)
    MCU := atmega32u4
    BOARD := BOARD_A_LEO
else ifeq ($(TARGETNAME), fw_pro_micro)
    MCU := atmega32u4
    BOARD := BOARD_A_PRO_MICRO
else ifeq ($(TARGETNAME), fw_dubfocus)
    MCU := atmega32u4
    BOARD := BOARD_DUBFOCUS
else ifeq ($(TARGETNAME), fw_bergamot)
    MCU := atmega32u4
    BOARD := BOARD_BERGAMOT
else ifeq ($(TARGETNAME), fw_teensy2pp)
    MCU := at90usb1286
    BOARD := BOARD_T_2PP
else ifeq ($(TARGETNAME), fw_mega)
    MCU := atmega2560
    BOARD := BOARD_A_MEGA
else ifeq ($(TARGETNAME), fw_uno)
    MCU := atmega328p
    BOARD := BOARD_A_UNO
else ifeq ($(TARGETNAME), fw_16u2)
    MCU := atmega16u2
    BOARD := BOARD_A_xu2
else ifeq ($(TARGETNAME), fw_8u2)
    MCU := atmega8u2
    BOARD := BOARD_A_xu2
else ifeq ($(TARGETNAME), fw_mega6mux)
    MCU := atmega2560
    BOARD := BOARD_A_MEGA6MUX
else
    $(error Invalid target specified)
endif

#mcu specific
ifeq ($(MCU),atmega32u4)
    EEPROM_SIZE := 1024
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),at90usb1286)
    EEPROM_SIZE := 4096
    FLASH_SIZE_START_ADDR := 0x98
    FLASH_SIZE_END_ADDR := 0x9C
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),atmega16u2)
    EEPROM_SIZE := 512
    BOOT_START_ADDR := 0x3000
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),atmega8u2)
    EEPROM_SIZE := 512
    DEFINES += UART_INTERFACES=1
    DEFINES += USB_SUPPORTED
else ifeq ($(MCU),atmega2560)
    EEPROM_SIZE := 4096
    DEFINES += UART_INTERFACES=2
else ifeq ($(MCU),atmega328p)
    EEPROM_SIZE := 1024
    DEFINES += UART_INTERFACES=1
endif

DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
DEFINES += EEPROM_SIZE=$(EEPROM_SIZE)
DEFINES += $(BOARD)

ifneq ($(HARDWARE_VERSION_MAJOR), )
    DEFINES += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
endif

ifneq ($(HARDWARE_VERSION_MINOR), )
    DEFINES += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)
endif