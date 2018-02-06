#includes
include Defines.mk
include LUFAsources.mk
include FirmwareSources.mk
include Upload.mk

#target/build location
BUILD_DIR := build/
TARGET := $(BUILD_DIR)$(MAKECMDGOALS)

#specify c/c++ compilers
CPP := avr-gcc
CXX := avr-g++

#common compiler flags
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

#common linker flags
LDFLAGS := -Wl,--gc-sections -mrelax -Wl,--start-group -Wl,-lm -Wl,--end-group

#optimisation level
ifneq ($(filter fw_%, $(MAKECMDGOALS)), )
    #firmware
    OPT := -O2
else
    #optimize for size for bootloader
    OPT := -Os
endif

LEN_APPEND := 0

#common include files
INCLUDE_FILES := -include "modules/core/src/Core.h"

ifneq ($(MAKECMDGOALS),fw_16u2)
    #unused on 16u2 firmware
    INCLUDE_FILES += -include "firmware/OpenDeck.h"
endif

#common include directories
INCLUDE_DIRS := \
-I"modules/lufa/" \
-I"modules/"

#custom linker script for 32u4/1286 boards
ifeq ($(MCU),atmega32u4)
    LDFLAGS += -T ld_32u4.x
    
    #append length only in firmware
    ifeq ($(findstring fw,$(MAKECMDGOALS)), fw)
        LEN_APPEND := 1
    endif
else ifeq ($(MCU),at90usb1286)
    LDFLAGS += -T ld_1286.x

    #append length only in firmware
    ifeq ($(findstring fw,$(MAKECMDGOALS)), fw)
        LEN_APPEND := 1
    endif
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
    CPP_OBJECTS_WDIR :=
    LDFLAGS += -Wl,--section-start=.text=$(BOOT_START_ADDR)
endif

#user sources
$(CPP_OBJECTS_WDIR): $(BUILD_DIR)%.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) -mmcu=$(MCU) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<

#lufa sources
$(LUFA_OBJS_WDIR): $(BUILD_DIR)%.o: %.c
	@mkdir -p $(@D)
	@$(CPP) $(CXXFLAGS) -std=gnu99 $(addprefix -D,$(DEFINES)) $(OPT) -mmcu=$(MCU) $(INCLUDE_DIRS) -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<

#determine if we're running make on Windows subsystem for Linux
#this is needed when running upload command since we can't access usb devices directly
#instead, call upload command from windows
ifeq ($(findstring Microsoft,$(shell uname -r)), Microsoft)
WINBASH := cmd.exe /c
#also, replace forward slash with backslash in path
AVR_DUDE_PATH := $(subst /,\,$(AVR_DUDE_PATH))
PATH_QUOTE := "
endif

#targets
fw_opendeck fw_leonardo fw_pro_micro fw_teensy2pp fw_mega fw_uno fw_16u2 boot_opendeck boot_leonardo boot_16u2 boot_teensy2pp: $(TARGET).elf

#determine if make clean should be run
#first check if MCU file in build dir exists
ifneq ("$(wildcard $(BUILD_DIR)MCU)","")
    ifneq ($(shell cat $(BUILD_DIR)MCU), $(MCU_avrdude))
        RUN_CLEAN := clean
    endif
endif

$(TARGET).elf: $(RUN_CLEAN) $(LUFA_OBJS_WDIR) $(CPP_OBJECTS_WDIR)
	@#compile first
	@$(CXX) -o$(TARGET).elf $(LUFA_OBJS_WDIR) $(CPP_OBJECTS_WDIR) -mmcu=$(MCU) $(LDFLAGS) -Wl,-Map,$(TARGET).map
	@echo Finished building target: $@
	@#convet elf to hex
	@avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "$(TARGET).elf" "$(TARGET).hex"
	@#write firmware length at specified location and crc at the end of compiled binary if supported for target
	@if [ "$(LEN_APPEND)" = "1" ]; then\
		srec_cat $(TARGET).hex -Intel -exclude $(FLASH_SIZE_START_ADDR) $(FLASH_SIZE_END_ADDR) -Little_Endian_Maximum $(FLASH_SIZE_START_ADDR) -fill 0xff -over $(TARGET).hex -I -Output $(TARGET).hex -Intel;\
		srec_cat $(TARGET).hex -Intel -Little_Endian_CRC16 -max-address $(TARGET).hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $(TARGET).hex -Intel;\
	fi
	@#convert hex to bin
	@objcopy -I ihex "$(TARGET).hex" -O binary "$(TARGET).bin"
	@#create a file with contents being mcu name (for upload process)
	@echo $(MCU_avrdude) > $(BUILD_DIR)MCU
	@#display memory usage
	@avr-size -C --mcu=$(MCU) "$(TARGET).elf"

#use existing makefile for mega bootloader
boot_mega:
	@mkdir -p build
	@cd bootloader/stk500v2 && $(MAKE) mega2560 --no-print-directory
	@cp bootloader/stk500v2/stk500boot_v2_mega2560.hex build/boot_mega.hex
	@cd bootloader/stk500v2 && $(MAKE) clean --no-print-directory

#firmware upload with avrdude
upload:
	@$(WINBASH) $(PATH_QUOTE)$(AVR_DUDE_PATH)$(AVR_DUDE_BIN)$(PATH_QUOTE) -p $(shell cat $(BUILD_DIR)MCU) -c $(AVRDUDE_PROGRAMMER) -e -U lock:w:$(FUSE_UNLOCK):m -U efuse:w:$(FUSE_EXT):m -U hfuse:w:$(FUSE_HIGH):m -U lfuse:w:$(FUSE_LOW):m
	@$(WINBASH) $(PATH_QUOTE)$(AVR_DUDE_PATH)$(AVR_DUDE_BIN)$(PATH_QUOTE) -p $(shell cat $(BUILD_DIR)MCU) -c $(AVRDUDE_PROGRAMMER) -U flash:w:$(shell find . -name "*.hex" | head -1) -U lock:w:$(FUSE_LOCK):m

#other targets
clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR)

#debugging
print-%:
	@echo '$*=$($*)'
