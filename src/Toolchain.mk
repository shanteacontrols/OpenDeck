#toolchain checks

ifeq ($(shell uname), Linux)
    FIND := find
else ifeq ($(shell uname), Darwin)
    FIND := gfind
else
    $(error Unsupported platform)
endif

C_COMPILER_AVR := avr-gcc
CPP_COMPILER_AVR := avr-g++
LINKER_AVR := avr-g++
FLASH_BIN_AVR := avrdude
C_COMPILER_ARM := arm-none-eabi-gcc
CPP_COMPILER_ARM := arm-none-eabi-g++
LINKER_ARM := arm-none-eabi-g++
FLASH_BIN_ARM := arm-none-eabi-gdb
C_COMPILER_x86 := clang
CPP_COMPILER_x86 := clang++
LINKER_x86 := clang++

CLANG_FORMAT := clang-format

REQ_PACKAGES := \
git \
$(FIND) \
jq \
yq \
srec_cat \
$(C_COMPILER_AVR) \
$(CPP_COMPILER_AVR) \
$(FLASH_BIN_AVR) \
$(C_COMPILER_ARM) \
$(CPP_COMPILER_ARM) \
$(FLASH_BIN_ARM) \
$(C_COMPILER_x86) \
$(CPP_COMPILER_x86)

#avoid find errors
#defined here to avoid verify target parsing "2>/dev/null" as an package causing it to fail
FIND := $(FIND) 2>/dev/null

verify:
	@echo Verifying all required packages...
	$(foreach package, $(REQ_PACKAGES), $(if $(shell which $(package) 2>/dev/null),,$(error Required package not found: $(package))))
	@echo All required packages found!