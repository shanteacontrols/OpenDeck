# Device Firmware Update utility

flash:
ifeq ($(shell uname), Linux)
	@if ! [ $$(id -u) = 0 ]; then \
	echo "dfu-util must be run as root user on Linux"; \
	false;
endif
	@sudo dfu-util -d 0483:df11 -a 0 -i 0 -s 0x8000000:leave -D $(FLASH_BINARY_DIR)/$(TARGET).bin