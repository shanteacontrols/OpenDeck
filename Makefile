include $(ZEPHYR_WS)/zenv/make/Makefile

PROJECT_PRESETS_FILE := $(ZEPHYR_PROJECT)/app/presets.yml
NON_TARGET_GOALS     := clean clean-all format print-%
OPENDECK_OVERLAYS    := $(sort $(wildcard $(ZEPHYR_PROJECT)/app/boards/opendeck/*/opendeck.overlay))
OPENDECK_TARGET_DIRS := $(patsubst %/,%,$(dir $(OPENDECK_OVERLAYS)))
OPENDECK_TARGETS     := $(notdir $(OPENDECK_TARGET_DIRS))
TARGET               := $(firstword $(OPENDECK_TARGETS))

ifeq ($(strip $(TARGET)),)
    $(error No OpenDeck targets found under $(ZEPHYR_PROJECT)/app/boards/opendeck)
endif

ifneq ($(filter $(TARGET),$(OPENDECK_TARGETS)),$(TARGET))
    $(error Unknown TARGET '$(TARGET)'. Available targets: $(OPENDECK_TARGETS))
endif

export TARGET

OPENDECK_OVERLAY := $(ZEPHYR_PROJECT)/app/boards/opendeck/$(TARGET)/opendeck.overlay
BUILD_DIR_APP    := $(BUILD_DIR_APP)/$(TARGET)
BUILD_DIR_TESTS  := $(BUILD_DIR_TESTS)/$(TARGET)
BOARD            := $(shell sed -n 's/.*zephyr-board[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' $(OPENDECK_OVERLAY) 2>/dev/null | head -n 1)
