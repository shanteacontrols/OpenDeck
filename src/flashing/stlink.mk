# STM32 Programmer utility on SWD interface

flash:
	@STM32_Programmer_CLI -c port=SWD sn=$(STLINK_SERIAL) -w $(FLASH_BINARY_DIR)/$(TARGET).hex -rst