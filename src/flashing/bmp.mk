# Black Magic Probe

flash:
ifeq ($(PORT),)
	$(error Serial port for BMP not specified (eg. PORT=ttyACM0))
endif
	@timeout 30 arm-none-eabi-gdb -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(FLASH_BINARY_DIR)/$(TARGET).hex | tee $(FLASH_BINARY_DIR)/gdb_output
	@if grep -q "MIS-MATCHED" $(FLASH_BINARY_DIR)/gdb_output; then \
	false; \
	fi