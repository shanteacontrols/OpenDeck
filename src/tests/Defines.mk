#slightly modified Defines.mk from src directory

#common
DEFINES :=

TARGETNAME := fw_opendeck
BOARD_DIR := $(subst fw_,,$(TARGETNAME))

#determine the architecture by directory in which the board dir is located
ifeq ($(findstring avr,$(shell find ../ -type d -name *$(BOARD_DIR))), avr)
    ARCH := avr
else ifeq ($(findstring stm32,$(shell find . -type d -name *$(BOARD_DIR))), stm32)
    ARCH := stm32
endif

#board specific
ifneq ($(shell cat ../board/$(ARCH)/variants/$(BOARD_DIR)/Hardware.h | grep atmega32u4), )
    MCU := atmega32u4
else ifneq ($(shell cat ../board/$(ARCH)/variants/$(BOARD_DIR)/Hardware.h | grep at90usb1286), )
    MCU := at90usb1286
else ifneq ($(shell cat ../board/$(ARCH)/variants/$(BOARD_DIR)/Hardware.h | grep atmega16u2), )
    MCU := atmega16u2
else ifneq ($(shell cat ../board/$(ARCH)/variants/$(BOARD_DIR)/Hardware.h | grep atmega8u2), )
    MCU := atmega8u2
else ifneq ($(shell cat ../board/$(ARCH)/variants/$(BOARD_DIR)/Hardware.h | grep atmega2560), )
    MCU := atmega2560
else ifneq ($(shell cat ../board/$(ARCH)/variants/$(BOARD_DIR)/Hardware.h | grep atmega328p), )
    MCU := atmega328p
else ifneq ($(shell cat ../board/$(ARCH)/variants/$(BOARD_DIR)/Hardware.h | grep stm32f407), )
    MCU := stm32f407
endif

#mcu specific
ifeq ($(MCU),atmega32u4)
    EEPROM_SIZE := 1024
    FLASH_SIZE_START_ADDR := 0xAC
else ifeq ($(MCU),at90usb1286)
    EEPROM_SIZE := 4096
    FLASH_SIZE_START_ADDR := 0x98
else ifeq ($(MCU),atmega16u2)
    EEPROM_SIZE := 512
    FLASH_SIZE_START_ADDR := 0x74
else ifeq ($(MCU),atmega8u2)
    EEPROM_SIZE := 512
    FLASH_SIZE_START_ADDR := 0x74
else ifeq ($(MCU),atmega2560)
    EEPROM_SIZE := 4096
    FLASH_SIZE_START_ADDR := 0xE4
else ifeq ($(MCU),atmega328p)
    EEPROM_SIZE := 1024
    FLASH_SIZE_START_ADDR := 0x68
endif

DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
DEFINES += EEPROM_SIZE=$(EEPROM_SIZE)
DEFINES += OD_BOARD_$(shell echo $(BOARD_DIR) | tr 'a-z' 'A-Z')

ifneq ($(HARDWARE_VERSION_MAJOR), )
    DEFINES += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
endif

ifneq ($(HARDWARE_VERSION_MINOR), )
    DEFINES += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)
endif