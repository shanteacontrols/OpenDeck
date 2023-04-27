SW_VERSION_MAJOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1)
SW_VERSION_MINOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2)
SW_VERSION_REVISION         := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3)

FW_METADATA_SIZE            := 4

# These can be overriden by target/vendor/arch/mcu etc.
BUFFER_SIZE_USB_OVER_SERIAL ?= 16
MIDI_SYSEX_ARRAY_SIZE       ?= 100

DEFINES += \
BUFFER_SIZE_USB_OVER_SERIAL=$(BUFFER_SIZE_USB_OVER_SERIAL) \
BUFFER_SIZE_TSCREEN_CDC_PASSTHROUGH=$(BUFFER_SIZE_USB_OVER_SERIAL) \
MIDI_SYSEX_ARRAY_SIZE=$(MIDI_SYSEX_ARRAY_SIZE) \
SW_VERSION_MAJOR=$(SW_VERSION_MAJOR) \
SW_VERSION_MINOR=$(SW_VERSION_MINOR) \
SW_VERSION_REVISION=$(SW_VERSION_REVISION)

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
endif

ifeq ($(LOG), 1)
    DEFINES += USE_LOGGER
endif

ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
    DEFINES += \
    BOARD_USE_UPDATE_HOOKS
endif