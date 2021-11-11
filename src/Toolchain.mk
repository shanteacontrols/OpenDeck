#toolchain checks

ifeq ($(shell uname), Linux)
    FIND      := find
    SHA256SUM := sha256sum
else ifeq ($(shell uname), Darwin)
    FIND      := gfind
    SHA256SUM := gsha256sum
else
    $(error Unsupported platform)
endif

C_COMPILER_AVR := avr-gcc
CPP_COMPILER_AVR := avr-g++
LINKER_AVR := avr-g++
FLASH_TOOL_AVR := arduino
C_COMPILER_ARM := arm-none-eabi-gcc
CPP_COMPILER_ARM := arm-none-eabi-g++
LINKER_ARM := arm-none-eabi-g++
FLASH_TOOL_ARM := bmp
C_COMPILER_NATIVE := gcc
CPP_COMPILER_NATIVE := g++
LINKER_NATIVE := g++

CLANG_FORMAT := clang-format
YAML_PARSER := dasel

REQ_PACKAGES := \
git \
$(FIND) \
srec_cat \
$(C_COMPILER_AVR) \
$(CPP_COMPILER_AVR) \
$(C_COMPILER_ARM) \
$(CPP_COMPILER_ARM) \
$(C_COMPILER_NATIVE) \
$(CPP_COMPILER_NATIVE) \
objcopy \
$(YAML_PARSER) \
$(SHA256SUM) \
avrdude

#don't allow running make at all if required packages don't exist on the system
#don't run this if the user is root - it's very likely sudo is used for flashing in this case
#and some of the binaries may not be available due to the PATH being set for non-root user only
ifneq ($(shell id -u), 0)
    $(foreach package, $(REQ_PACKAGES), $(if $(shell which $(package) 2>/dev/null),,$(error Required package not found: $(package))))
endif

#avoid find errors
#defined here to avoid verify target parsing "2>/dev/null" as an package causing it to fail
FIND := $(FIND) 2>/dev/null

#add flags here to avoid manual typing everywhere
YAML_PARSER := $(YAML_PARSER) -n -p yaml --plain -f