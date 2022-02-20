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

CC_AVR          := avr-gcc
CXX_AVR         := avr-g++
LD_AVR          := avr-g++
CC_ARM          := arm-none-eabi-gcc
CXX_ARM         := arm-none-eabi-g++
LD_ARM          := arm-none-eabi-g++
CC_NATIVE       := gcc
CXX_NATIVE      := g++
LD_NATIVE       := g++
CLANG_FORMAT    := clang-format
YAML_PARSER     := dasel

REQ_PACKAGES := \
git \
$(FIND) \
srec_cat \
$(CC_AVR) \
$(CXX_AVR) \
$(CC_ARM) \
$(CXX_ARM) \
$(CC_NATIVE) \
$(CXX_NATIVE) \
objcopy \
$(YAML_PARSER) \
$(SHA256SUM) \
ccache

#don't allow running make at all if required packages don't exist on the system
#don't run this if the user is root - it's very likely sudo is used for flashing in this case
#and some of the binaries may not be available due to the PATH being set for non-root user only
ifneq ($(shell id -u), 0)
    $(foreach package, $(REQ_PACKAGES), $(if $(shell which $(package) 2>/dev/null),,$(error Required package not found: $(package))))
endif

#avoid find errors
#defined here to avoid verify target parsing "2>/dev/null" as an package causing it to fail
FIND := $(FIND) 2>/dev/null