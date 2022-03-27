NRF52_SDK_VERSION        := nRF5_SDK_17.1.0_ddde560
NRF52_SDK_LINK           := https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/$(NRF52_SDK_VERSION).zip
NRF52_SDK_FILENAME       := $(shell basename $(NRF52_SDK_LINK))
NRF52_SDK_ZIP_PATH       := $(DEPS_DIR)/$(NRF52_SDK_FILENAME)
NRF52_SOFTDEVICE_PATH    := $(DEPS_DIR)/$(shell basename $(NRF52_SDK_FILENAME) .zip)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex
NRF52_SDK_DL_MARKER_PATH := $(NRF52_SDK_ZIP_PATH).downloaded

ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_FILES += \
    -include "board/arch/arm/nordic/variants/nrf52/nrfx_config.h"

    INCLUDE_DIRS += \
    -I"deps/$(NRF52_SDK_VERSION)/components/softdevice/s140/headers" \
    -I"deps/$(NRF52_SDK_VERSION)/components/softdevice/s140/headers/nrf52"
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