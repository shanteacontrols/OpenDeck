include Defines.mk
include LUFAsources.mk
include FirmwareSources.mk

########################
#compiler options
CPP := avr-gcc
CXX := avr-g++

CXXFLAGS := \
-funsigned-char \
-funsigned-bitfields \
-fdata-sections \
-ffunction-sections \
-fpack-struct \
-fshort-enums \
-mrelax \
-Wall \
-c \
-fno-jump-tables \
-fno-strict-aliasing

LDFLAGS := -Wl,--gc-sections -mrelax -Wl,--start-group -Wl,-lm -Wl,--end-group

#optimisation for firmware - bootloader uses -Os
OPT := -O2

########################

BUILD_DIR := build/

INCLUDE_FILES := -include "modules/core/src/Core.h"
INCLUDE_DIRS := \
-I"modules/lufa/" \
-I"modules/"

### configuration specific
ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)
#use -Os for bootloader
OPT := -Os
endif
######

ifneq ($(MAKECMDGOALS),fw_16u2)
INCLUDE_FILES += -include "firmware/OpenDeck.h"
endif

ifneq ($(LUFA_OBJS),)
#only do this if lufa objects exist
LUFA_OBJS_WDIR := $(addprefix $(BUILD_DIR),$(LUFA_OBJS))
else
LUFA_OBJS_WDIR :=
endif

CPP_OBJECTS_WDIR := $(addprefix $(BUILD_DIR),$(CPP_OBJECTS))

ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)
#don't use cpp sources
CPP_OBJECTS :=
CPP_OBJECTS_WDIR :=
LDFLAGS += -Wl,--section-start=.text=$(BOOT_START_ADDR)
endif

#user start
$(CPP_OBJECTS_WDIR): $(BUILD_DIR)%.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(OPT) -mmcu=$(MCU) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<
#user end

#lufa start
$(LUFA_OBJS_WDIR): $(BUILD_DIR)%.o: %.c
	@mkdir -p $(@D)
	@$(CPP) $(CXXFLAGS) -std=gnu99 $(DEFINES) $(OPT) -mmcu=$(MCU) $(INCLUDE_DIRS) -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<
#lufa end

#targets
fw_opendeck fw_leonardo fw_mega fw_16u2 boot_opendeck boot_leonardo boot_16u2: $(MAKECMDGOALS).elf

$(MAKECMDGOALS).elf: $(LUFA_OBJS_WDIR) $(CPP_OBJECTS_WDIR)
	@$(CXX) -o$(MAKECMDGOALS).elf $(LUFA_OBJS_WDIR) $(CPP_OBJECTS_WDIR) -mmcu=$(MCU) $(LDFLAGS)
	@echo Finished building target: $@
	@avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "$(MAKECMDGOALS).elf" "../bin/compiled/$(MAKECMDGOALS).hex"
	@avr-size "$(MAKECMDGOALS).elf"
	@rm $(MAKECMDGOALS).elf

# Other Targets
clean:
	rm -rf *.elf
	rm -rf $(BUILD_DIR)

print-%:
	@echo '$*=$($*)'