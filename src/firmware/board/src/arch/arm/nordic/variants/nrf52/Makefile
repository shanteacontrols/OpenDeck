NRF52_SDK_PATH           := $(shell $(FIND) $(ROOT_MAKEFILE_DIR)/../modules/core/deps/nrf52840 -maxdepth 1 -type d -name "*_SDK_*")
NRF52_SOFTDEVICE_PATH    := $(NRF52_SDK_PATH)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex

DEFINES += \
SOFTDEVICE_PRESENT

post-build::
	$(Q)echo -n "Appending SoftDevice to merged binary..."
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel \
	$(NRF52_SOFTDEVICE_PATH) -Intel \
	-o $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex -Intel
	$(Q)objcopy -I ihex "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).bin"
	$(Q)echo " done"