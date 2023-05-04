SW_VERSION_MAJOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1)
SW_VERSION_MINOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2)
SW_VERSION_REVISION         := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3)

FW_METADATA_SIZE            := 4
MIDI_SYSEX_ARRAY_SIZE       ?= 100

DEFINES += \
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