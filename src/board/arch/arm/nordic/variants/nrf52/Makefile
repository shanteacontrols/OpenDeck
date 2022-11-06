NRF52_SDK_VERSION        := 17.1.0_ddde560
NRF52_SDK_LINK           := https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5_sdk_$(NRF52_SDK_VERSION).zip
NRF52_SDK_FILENAME       := $(shell basename $(NRF52_SDK_LINK))
NRF52_SDK_ZIP_PATH       := $(DEPS_DIR)/$(NRF52_SDK_FILENAME)
NRF52_SDK_DL_MARKER_PATH := $(NRF52_SDK_ZIP_PATH).downloaded
NRF52_SDK_PATH           := $(shell $(FIND) $(DEPS_DIR) -maxdepth 1 -type d -iname "*nrf*")
NRF52_SOFTDEVICE_PATH    := $(NRF52_SDK_PATH)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex

ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_FILES += \
    -include "board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common/nrfx_config.h"

    INCLUDE_DIRS += \
    -I"$(NRF52_SDK_PATH)/components/softdevice/s140/headers" \
    -I"$(NRF52_SDK_PATH)/components/softdevice/s140/headers/nrf52" \
    -I"$(NRF52_SDK_PATH)/components/softdevice/common" \
    -I"$(NRF52_SDK_PATH)/components/libraries/util" \
    -I"$(NRF52_SDK_PATH)/components/libraries/experimental_section_vars" \
    -I"$(NRF52_SDK_PATH)/components/libraries/log" \
    -I"$(NRF52_SDK_PATH)/components/libraries/log/src" \
    -I"$(NRF52_SDK_PATH)/components/libraries/strerror" \
    -I"$(NRF52_SDK_PATH)/components/ble/nrf_ble_qwr" \
    -I"$(NRF52_SDK_PATH)/components/ble/nrf_ble_gatt" \
    -I"$(NRF52_SDK_PATH)/components/ble/common" \
    -I"$(NRF52_SDK_PATH)/components/ble/ble_advertising" \
    -I"$(NRF52_SDK_PATH)/components/libraries/timer" \
    -I"$(NRF52_SDK_PATH)/components/libraries/atomic" \
    -I"$(NRF52_SDK_PATH)/components/libraries/delay" \
    -I"$(NRF52_SDK_PATH)/components/ble/peer_manager" \
    -I"$(NRF52_SDK_PATH)/components/libraries/fds" \
    -I"$(NRF52_SDK_PATH)/components/libraries/atomic_fifo" \
    -I"$(NRF52_SDK_PATH)/components/libraries/fstorage" \
    -I"$(NRF52_SDK_PATH)/components/libraries/atomic_flags" \
    -I"$(NRF52_SDK_PATH)/components/libraries/mutex" \
    -I"$(NRF52_SDK_PATH)/modules/nrfx/hal"

    SOURCES += \
    $(NRF52_SDK_PATH)/components/libraries/experimental_section_vars/nrf_section_iter.c \
    $(NRF52_SDK_PATH)/components/libraries/atomic_fifo/nrf_atfifo.c \
    $(NRF52_SDK_PATH)/components/libraries/atomic/nrf_atomic.c \
    $(NRF52_SDK_PATH)/components/libraries/util/app_error.c \
    $(NRF52_SDK_PATH)/components/libraries/util/app_error_weak.c \
    $(NRF52_SDK_PATH)/components/libraries/util/app_error_handler_gcc.c \
    $(NRF52_SDK_PATH)/components/libraries/util/app_util_platform.c \
    $(NRF52_SDK_PATH)/components/softdevice/common/nrf_sdh_ble.c \
    $(NRF52_SDK_PATH)/components/softdevice/common/nrf_sdh_soc.c \
    $(NRF52_SDK_PATH)/components/softdevice/common/nrf_sdh.c \
    $(NRF52_SDK_PATH)/components/libraries/timer/app_timer.c

    SOURCES += $(shell $(FIND) $(NRF52_SDK_PATH)/components/libraries/fstorage -type f -name "*.c")

    ifeq ($(TYPE),application)
        SOURCES += \
        $(NRF52_SDK_PATH)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c \
        $(NRF52_SDK_PATH)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
        $(NRF52_SDK_PATH)/components/ble/ble_advertising/ble_advertising.c \
        $(NRF52_SDK_PATH)/components/ble/common/ble_advdata.c \
        $(NRF52_SDK_PATH)/components/ble/common/ble_conn_params.c \
        $(NRF52_SDK_PATH)/components/ble/common/ble_srv_common.c \
        $(NRF52_SDK_PATH)/components/libraries/fds/fds.c \
        $(NRF52_SDK_PATH)/components/ble/common/ble_conn_state.c \
        $(NRF52_SDK_PATH)/components/libraries/atomic_flags/nrf_atflags.c \

        SOURCES += $(shell $(FIND) $(NRF52_SDK_PATH)/components/ble/peer_manager -type f -name "*.c")
        SOURCES += $(shell $(FIND) board/arch/arm/nordic/variants/nrf52/ble -type f -name "*.cpp")
    endif
endif

DEFINES += \
SOFTDEVICE_PRESENT

$(NRF52_SDK_DL_MARKER_PATH):
	$(Q)echo Downloading nRF52 SDK...
	$(Q)mkdir -p $(DEPS_DIR)
	$(Q)wget --quiet $(NRF52_SDK_LINK) -P $(DEPS_DIR)
	$(Q)echo Extracting nRF52 SDK...
	$(Q)unzip -q $(NRF52_SDK_ZIP_PATH) -d $(DEPS_DIR)
	$(Q)touch $(NRF52_SDK_DL_MARKER_PATH)
	$(Q)rm $(NRF52_SDK_ZIP_PATH)

pre-build:: $(NRF52_SDK_DL_MARKER_PATH)

post-build::
	$(Q)echo -n "Appending SoftDevice to merged binary..."
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel \
	$(NRF52_SOFTDEVICE_PATH) -Intel \
	-o $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel
	$(Q)objcopy -I ihex "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).bin"
	$(Q)echo " done"