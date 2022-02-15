# J-Link through nrfjprog tool

flash:
	@nrfjprog -f nrf52 --program $(FLASH_BINARY_DIR)/$(TARGET).hex --sectorerase
	@nrfjprog -f nrf52 --reset