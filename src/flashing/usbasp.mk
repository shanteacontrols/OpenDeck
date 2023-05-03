# avrdude with usbasp configuration

flash:
	@avrdude -p $(CORE_MCU_MODEL) -P usb -c usbasp -C /etc/avrdude.conf -e -V -u -U lock:w:$(PROJECT_MCU_FUSE_UNLOCK):m -U efuse:w:$(PROJECT_MCU_FUSE_EXT):m -U hfuse:w:$(PROJECT_MCU_FUSE_HIGH):m -U lfuse:w:$(PROJECT_MCU_FUSE_LOW):m
	@avrdude -p $(CORE_MCU_MODEL) -P usb -c usbasp -C /etc/avrdude.conf -U flash:w:$(FLASH_BINARY_DIR)/$(TARGET).hex
	@avrdude -p $(CORE_MCU_MODEL) -P usb -c usbasp -C /etc/avrdude.conf -V -u -U lock:w:$(PROJECT_MCU_FUSE_LOCK):m