# Helper function used to verify if given list of packages exist on the system.
# Don't run this if the user is root - it's very likely sudo is used for flashing in this case
# and some of the binaries may not be available due to the PATH being set for non-root user only.
ifneq ($(shell id -u), 0)
define CHECK_PACKAGES
	$(foreach package, $(1), $(if $(shell which $(package) 2>/dev/null),,$(error Required package not found: $(package))))
endef
endif

ifeq ($(shell uname), Linux)
    FIND      := find
    SHA256SUM := sha256sum
else ifeq ($(shell uname), Darwin)
    FIND      := gfind
    SHA256SUM := gsha256sum
else
    $(error Unsupported platform)
endif

CC_NATIVE       := gcc
CXX_NATIVE      := g++
LD_NATIVE       := g++
CLANG_FORMAT    := clang-format
YAML_PARSER     := dasel
CCACHE          := ccache

REQ_PACKAGES := \
awk \
git \
$(FIND) \
srec_cat \
$(CC_NATIVE) \
$(CXX_NATIVE) \
objcopy \
$(YAML_PARSER) \
$(SHA256SUM) \
$(CCACHE)

$(call CHECK_PACKAGES,$(REQ_PACKAGES))

# Redefine find here to avoid verify target parsing "2>/dev/null" as an package causing it to fail.
# Used to avoid build failures if some sources don't exist for a given target.
FIND := $(FIND) 2>/dev/null