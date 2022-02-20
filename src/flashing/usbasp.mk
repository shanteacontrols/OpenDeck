# avrdude with usbasp configuration

flash:
	@avrdude -p $(MCU) -P usb -c usbasp -C /etc/avrdude.conf -e -V -u -U lock:w:$(FUSE_UNLOCK):m -U efuse:w:$(FUSE_EXT):m -U hfuse:w:$(FUSE_HIGH):m -U lfuse:w:$(FUSE_LOW):m
	@avrdude -p $(MCU) -P usb -c usbasp -C /etc/avrdude.conf -U flash:w:$(FLASH_BINARY_DIR)/$(TARGET).hex
	@avrdude -p $(MCU) -P usb -c usbasp -C /etc/avrdude.conf -V -u -U lock:w:$(FUSE_LOCK):m