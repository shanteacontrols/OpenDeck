#toolchain checks

ifeq ($(shell uname), Linux)
    FIND := find
else ifeq ($(shell uname), Darwin)
    FIND := gfind
else
    $(error Unsupported platform)
endif

#use windows binary on wsl since HID access isn't possible in wsl
ifeq ($(findstring Microsoft,$(shell uname -r)), Microsoft)
    DFU_BIN := cmd.exe /c "..\bin\dfu\hid_bootloader_loader_win.exe"
else ifeq ($(shell uname), Linux)
    DFU_BIN := ../bin/dfu/hid_bootloader_loader_linux
else ifeq ($(shell uname), Darwin)
    DFU_BIN := ../bin/dfu/hid_bootloader_loader_mac
else
    $(error Unsupported platform)
endif

C_COMPILER_AVR := avr-gcc
CPP_COMPILER_AVR := avr-g++
FLASH_BIN_AVR := avrdude
C_COMPILER_ARM := arm-none-eabi-gcc
CPP_COMPILER_ARM := arm-none-eabi-g++
FLASH_BIN_ARM := arm-none-eabi-gdb

CLANG_FORMAT := clang-format

undefine override REQ_PACKAGES

REQ_PACKAGES := \
git \
$(FIND) \
$(DFU_BIN) \
jq \
srec_cat \
$(C_COMPILER_AVR) \
$(CPP_COMPILER_AVR) \
$(FLASH_BIN_AVR) \
$(C_COMPILER_ARM) \
$(CPP_COMPILER_ARM) \
$(FLASH_BIN_ARM) 

verify:
	@echo Verifying all required packages...
	$(foreach package, $(REQ_PACKAGES), $(if $(shell which $(package) 2>/dev/null),,$(error Required package not found: $(package))))