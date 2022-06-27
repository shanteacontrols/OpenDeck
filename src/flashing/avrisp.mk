# avrdude with AVRISP MK II configuration

flash:
	@avrdude -p $(CORE_MCU_MODEL) -P usb -c avrispmkII -C /etc/avrdude.conf -e -V -u -U lock:w:$(FUSE_UNLOCK):m -U efuse:w:$(FUSE_EXT):m -U hfuse:w:$(FUSE_HIGH):m -U lfuse:w:$(FUSE_LOW):m
	@avrdude -p $(CORE_MCU_MODEL) -P usb -c avrispmkII -C /etc/avrdude.conf -U flash:w:$(FLASH_BINARY_DIR)/$(TARGET).hex
	@avrdude -p $(CORE_MCU_MODEL) -P usb -c avrispmkII -C /etc/avrdude.conf -V -u -U lock:w:$(FUSE_LOCK):m