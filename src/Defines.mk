SW_VERSION_MAJOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1)
SW_VERSION_MINOR            := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2)
SW_VERSION_REVISION         := $(shell git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3)

DEFINES += \
SW_VERSION_MAJOR=$(SW_VERSION_MAJOR) \
SW_VERSION_MINOR=$(SW_VERSION_MINOR) \
SW_VERSION_REVISION=$(SW_VERSION_REVISION)

ifeq ($(DEBUG), 1)
    DEFINES += DEBUG
endif

ifeq ($(LOG), 1)
    DEFINES += APP_USE_LOGGER
    DEFINES += BOARD_USE_LOGGER
endif