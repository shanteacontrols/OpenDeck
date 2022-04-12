NRF52_SDK_VERSION        := nRF5_SDK_17.1.0_ddde560
NRF52_SDK_LINK           := https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/$(NRF52_SDK_VERSION).zip
NRF52_SDK_FILENAME       := $(shell basename $(NRF52_SDK_LINK))
NRF52_SDK_ZIP_PATH       := $(DEPS_DIR)/$(NRF52_SDK_FILENAME)
NRF52_SOFTDEVICE_PATH    := $(DEPS_DIR)/$(shell basename $(NRF52_SDK_FILENAME) .zip)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex
NRF52_SDK_DL_MARKER_PATH := $(NRF52_SDK_ZIP_PATH).downloaded

ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_FILES += \
    -include "board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY)/common/nrfx_config.h"

    INCLUDE_DIRS += \
    -I"deps/$(NRF52_SDK_VERSION)/components/softdevice/s140/headers" \
    -I"deps/$(NRF52_SDK_VERSION)/components/softdevice/s140/headers/nrf52" \
    -I"deps/$(NRF52_SDK_VERSION)/components/softdevice/common" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/util" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/experimental_section_vars" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/log" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/log/src" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/strerror" \
    -I"deps/$(NRF52_SDK_VERSION)/components/ble/nrf_ble_qwr" \
    -I"deps/$(NRF52_SDK_VERSION)/components/ble/nrf_ble_gatt" \
    -I"deps/$(NRF52_SDK_VERSION)/components/ble/common" \
    -I"deps/$(NRF52_SDK_VERSION)/components/ble/ble_advertising" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/timer" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/atomic" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/delay" \
    -I"deps/$(NRF52_SDK_VERSION)/components/ble/peer_manager" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/fds" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/atomic_fifo" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/fstorage" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/atomic_flags" \
    -I"deps/$(NRF52_SDK_VERSION)/components/libraries/mutex" \
    -I"deps/$(NRF52_SDK_VERSION)/modules/nrfx/hal"

    SOURCES += \
    deps/$(NRF52_SDK_VERSION)/components/libraries/experimental_section_vars/nrf_section_iter.c \
    deps/$(NRF52_SDK_VERSION)/components/libraries/atomic_fifo/nrf_atfifo.c \
    deps/$(NRF52_SDK_VERSION)/components/libraries/atomic/nrf_atomic.c \
    deps/$(NRF52_SDK_VERSION)/components/libraries/util/app_error.c \
    deps/$(NRF52_SDK_VERSION)/components/libraries/util/app_error_weak.c \
    deps/$(NRF52_SDK_VERSION)/components/libraries/util/app_error_handler_gcc.c \
    deps/$(NRF52_SDK_VERSION)/components/libraries/util/app_util_platform.c \
    deps/$(NRF52_SDK_VERSION)/components/softdevice/common/nrf_sdh_ble.c \
    deps/$(NRF52_SDK_VERSION)/components/softdevice/common/nrf_sdh_soc.c \
    deps/$(NRF52_SDK_VERSION)/components/softdevice/common/nrf_sdh.c \
    deps/$(NRF52_SDK_VERSION)/components/libraries/timer/app_timer.c

    SOURCES += $(shell $(FIND) deps/$(NRF52_SDK_VERSION)/components/libraries/fstorage -type f -name "*.c")

    ifeq ($(TYPE),app)
        SOURCES += \
        deps/$(NRF52_SDK_VERSION)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c \
        deps/$(NRF52_SDK_VERSION)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
        deps/$(NRF52_SDK_VERSION)/components/ble/ble_advertising/ble_advertising.c \
        deps/$(NRF52_SDK_VERSION)/components/ble/common/ble_advdata.c \
        deps/$(NRF52_SDK_VERSION)/components/ble/common/ble_conn_params.c \
        deps/$(NRF52_SDK_VERSION)/components/ble/common/ble_srv_common.c \
        deps/$(NRF52_SDK_VERSION)/components/libraries/fds/fds.c \
        deps/$(NRF52_SDK_VERSION)/components/ble/common/ble_conn_state.c \
        deps/$(NRF52_SDK_VERSION)/components/libraries/atomic_flags/nrf_atflags.c \

        SOURCES += $(shell $(FIND) deps/$(NRF52_SDK_VERSION)/components/ble/peer_manager -type f -name "*.c")
        SOURCES += $(shell $(FIND) board/arch/arm/nordic/variants/nrf52/ble -type f -name "*.cpp")
    endif
endif

DEFINES += \
SOFTDEVICE_PRESENT

$(NRF52_SDK_DL_MARKER_PATH):
	@echo Downloading nRF52 SDK...
	@mkdir -p $(DEPS_DIR)
	@wget --quiet $(NRF52_SDK_LINK) -P $(DEPS_DIR)
	@echo Extracting nRF52 SDK...
	@unzip -q $(NRF52_SDK_ZIP_PATH) -d $(DEPS_DIR)
	@touch $(NRF52_SDK_DL_MARKER_PATH)
	@rm $(NRF52_SDK_ZIP_PATH)

pre-build:: $(NRF52_SDK_DL_MARKER_PATH)

post-build::
	@echo -n "Appending SoftDevice to merged binary..."
	@srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel \
	$(NRF52_SOFTDEVICE_PATH) -Intel \
	-o $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel
	@objcopy -I ihex "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).bin"
	@echo " done"