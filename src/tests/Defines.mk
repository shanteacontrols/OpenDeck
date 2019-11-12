#slightly modified Defines.mk from src directory

#common
DEFINES :=

TARGETNAME := fw_opendeck
BOARD_DIR := $(subst fw_,,$(TARGETNAME))

#determine the architecture by directory in which the board dir is located
ARCH := $(shell find ../board/ -type d ! -path *build -name *$(BOARD_DIR) | cut -c 10- | cut -d/ -f1 | head -n 1)

#determine MCU by directory in which the board dir is located
MCU := $(shell find ../board/ -type d -name *$(BOARD_DIR) | cut -c 10- | cut -d/ -f3 | head -n 1)

#mcu specific
ifeq ($(MCU),atmega32u4)
    FLASH_SIZE_START_ADDR := 0xAC
else ifeq ($(MCU),at90usb1286)
    FLASH_SIZE_START_ADDR := 0x98
else ifeq ($(MCU),atmega16u2)
    FLASH_SIZE_START_ADDR := 0x74
else ifeq ($(MCU),atmega8u2)
    FLASH_SIZE_START_ADDR := 0x74
else ifeq ($(MCU),atmega2560)
    FLASH_SIZE_START_ADDR := 0xE4
else ifeq ($(MCU),atmega328p)
    FLASH_SIZE_START_ADDR := 0x68
endif

DEFINES += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
DEFINES += OD_BOARD_$(shell echo $(BOARD_DIR) | tr 'a-z' 'A-Z')

ifneq ($(HARDWARE_VERSION_MAJOR), )
    DEFINES += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
endif

ifneq ($(HARDWARE_VERSION_MINOR), )
    DEFINES += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)
endif