ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_FILES += \
    -include "board/arch/arm/nordic/variants/nrf52/nrfx_config.h"
endif

SOFT_DEVICE_DL_DIR := deps/sd

$(SOFT_DEVICE_DL_DIR)/s140_nrf52_7.3.0_softdevice.hex:
	@echo Downloading s140_nrf52_7.3.0...
	@mkdir -p $(SOFT_DEVICE_DL_DIR)
	@wget --quiet https://www.nordicsemi.com/-/media/Software-and-other-downloads/SoftDevices/S140/s140_nrf52_7.3.0.zip -O $(SOFT_DEVICE_DL_DIR)/sd.zip
	@unzip -q $(SOFT_DEVICE_DL_DIR)/sd.zip -d $(SOFT_DEVICE_DL_DIR)

post-build:: $(SOFT_DEVICE_DL_DIR)/s140_nrf52_7.3.0_softdevice.hex
	@echo -n "Appending SoftDevice to merged binary..."
	@srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel \
	$(SOFT_DEVICE_DL_DIR)/s140_nrf52_7.3.0_softdevice.hex -Intel \
	-o $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel
	@objcopy -I ihex "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).bin"
	@echo " done"