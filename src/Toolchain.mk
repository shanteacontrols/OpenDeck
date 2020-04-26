#toolchain checks

ifeq ($(shell uname), Linux)
    FIND := find
else ifeq ($(shell uname), Darwin)
    FIND := gfind
else
    $(error Unsupported platform)
endif

#avoid find errors
FIND := $(FIND) 2>/dev/null

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