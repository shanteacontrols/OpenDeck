# update firmware through opendeck bootloader

flash:
ifeq ($(shell amidi -l | grep "OpenDeck DFU | $(TARGET)"),)
$(error No OpenDeck DFU interface found)
endif
	@echo Sending firmware to device...
	@PORT=$(shell amidi -l | grep "OpenDeck DFU | $(TARGET)" | grep -Eo 'hw:\S*') && \
	amidi -p $$PORT -s $(FLASH_BINARY_DIR)/$(TARGET).sysex.syx
