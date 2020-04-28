include Toolchain.mk

BUILD_DIR_BASE := ./build
TARGETNAME := mega2560

ifneq (,$(wildcard $(BUILD_DIR_BASE)/TARGET))
    TARGETNAME := $(shell cat $(BUILD_DIR_BASE)/TARGET)
    DEBUG := $(shell cat $(BUILD_DIR_BASE)/DEBUG)
endif

ifneq ($(findstring $(TARGETNAME), $(shell jq ".targets[] | .name" ../targets.json)), $(TARGETNAME))
    $(error Target doesn't exist)
endif

ifeq ($(BOOT),1)
    #verify if target has bootloader
    ifeq ($(shell jq '.targets[] | select(.name=="$(TARGETNAME)") | .bootloader' ../targets.json), false)
        $(error This target doesn't have bootloader)
    endif

    BUILD_DIR := $(BUILD_DIR_BASE)/bootloader
else
    BUILD_DIR := $(BUILD_DIR_BASE)/application
endif

BUILD_DIR := $(BUILD_DIR)/$(TARGETNAME)

ifeq ($(DEBUG),1)
    BUILD_DIR := $(BUILD_DIR)/debug
else
    BUILD_DIR := $(BUILD_DIR)/release
endif

TARGET := $(BUILD_DIR)/$(TARGETNAME)
.DEFAULT_GOAL := $(TARGET).elf

#includes
#important - do not change the order of inclusion!
include Defines.mk
include Sources.mk

# shell script "build_combined.sh" can generate binary with combined firmware and bootloader
# if this file exists, use it when running flash target
MERGED_TARGET := $(shell $(FIND) $(BUILD_DIR_BASE)/merged -name "*$(BOARD_DIR).hex" 2>/dev/null)

#when set to 1, format target will fail if there are any changes to the repository after formatting
CF_FAIL_ON_DIFF := 0

#passed to both c and c++ compiler
COMMON_FLAGS := \
-Wall \
-fdata-sections \
-ffunction-sections \
-fmessage-length=0 \
-fno-strict-aliasing

#c++ compiler only
CPP_FLAGS := \
-std=c++11 \
-fno-rtti \
-fno-exceptions \
-fpermissive

#c compiler only
C_FLAGS := \
-std=c11

#assembler only
ASM_FLAGS := \
 -x assembler-with-cpp

#common linker flags
LDFLAGS := \
-Wl,--gc-sections \
-Wl,-Map="$(TARGET).map",--cref

ifeq ($(ARCH),avr)
    ifeq ($(BOOT),1)
        #make sure to link .text at correct address in bootloader
        LDFLAGS += -Wl,--section-start=.text=$(BOOT_START_ADDR)
    else
        #append length only in firmware
        LEN_APPEND := 1
    endif

    C_COMPILER := $(C_COMPILER_AVR)
    CPP_COMPILER := $(CPP_COMPILER_AVR)
    FLASH_BIN := $(FLASH_BIN_AVR)
    OPT := -Os

    C_FLAGS += \
    -fpack-struct

    COMMON_FLAGS += \
    -mrelax \
    -mmcu=$(MCU) \
    -funsigned-char \
    -funsigned-bitfields \
    -fshort-enums \
    -fno-jump-tables \
    -flto

    LDFLAGS += \
    -mrelax \
    -mmcu=$(MCU) \
    -flto
else ifeq ($(ARCH),stm32)
    C_COMPILER := $(C_COMPILER_ARM)
    CPP_COMPILER := $(CPP_COMPILER_ARM)
    ASSEMBLER := $(C_COMPILER_ARM)
    FLASH_BIN := $(FLASH_BIN_ARM)
    OPT := -O2

    COMMON_FLAGS += \
    -mcpu=$(CPU) \
    -mthumb \
    -mno-unaligned-access

    LDFLAGS += \
    -mcpu=$(CPU) \
    -mthumb \
    -specs=nano.specs \
    -specs=nosys.specs

    ifeq ($(DEBUG), 1)
        COMMON_FLAGS += -g -gdwarf-2
        LDFLAGS += -g -gdwarf-2
        OPT := -Og
    endif

    ifneq ($(FPU),)
        COMMON_FLAGS += -mfpu=$(FPU)
        LDFLAGS += -mfpu=$(FPU)
    endif

    ifneq ($(FLOAT-ABI),)
        COMMON_FLAGS += -mfloat-abi=$(FLOAT-ABI)
        LDFLAGS += -mfloat-abi=$(FLOAT-ABI)
    endif
else
    $(error Unsupported architecture)
endif

#custom linker script
LDFLAGS += -T $(LINKER_FILE)

$(TARGET).elf: $(OBJECTS)
	@echo $(TARGETNAME) > $(BUILD_DIR_BASE)/TARGET
	@echo $(DEBUG) > $(BUILD_DIR_BASE)/DEBUG
	@echo Creating executable: $@
	@$(CPP_COMPILER) -o$(TARGET).elf $(OBJECTS) $(LDFLAGS)
ifeq ($(ARCH), avr)
	@#convert elf to hex
	@avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "$(TARGET).elf" "$(TARGET).hex"
	@#write firmware length at specified location and crc at the end of compiled binary if supported for target
	@if [ "$(LEN_APPEND)" = "1" ]; then\
		srec_cat $(TARGET).hex -Intel -exclude $(FLASH_SIZE_START_ADDR) $(FLASH_SIZE_END_ADDR) -Little_Endian_Maximum $(FLASH_SIZE_START_ADDR) -fill 0xff -over $(TARGET).hex -I -Output $(TARGET).hex -Intel;\
		srec_cat $(TARGET).hex -Intel -Little_Endian_CRC16 -max-address $(TARGET).hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $(TARGET).hex -Intel;\
	fi
	@#convert hex to bin
	@avr-objcopy -I ihex "$(TARGET).hex" -O binary "$(TARGET).bin"
	@#display memory usage
	@avr-size -C --mcu=$(MCU) "$(TARGET).elf"
else
	@#convert elf to hex
	@arm-none-eabi-objcopy -O ihex $(TARGET).elf $(TARGET).hex
	@#convert hex to bin
	@arm-none-eabi-objcopy -I ihex "$(TARGET).hex" -O binary "$(TARGET).bin"
	@arm-none-eabi-size "$(TARGET).elf"
endif

pre-build:
	@../scripts/gen_touchscreen_indicators.sh application/io/touchscreen/coordinates/$(BOARD_DIR).json

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@echo Building: $<
	@$(C_COMPILER) $(COMMON_FLAGS) $(C_FLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@echo Building: $<
	@$(CPP_COMPILER) $(COMMON_FLAGS) $(CPP_FLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(@D)
	@echo Building: $<
	@$(ASSEMBLER) $(COMMON_FLAGS) $(C_FLAGS) $(ASM_FLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

flash:
ifeq ($(ARCH), avr)
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -e -V -u -U lock:w:$(FUSE_UNLOCK):m -U efuse:w:$(FUSE_EXT):m -U hfuse:w:$(FUSE_HIGH):m -U lfuse:w:$(FUSE_LOW):m
ifneq (,$(MERGED_TARGET))
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:w:$(MERGED_TARGET)
else
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:w:$(TARGET).hex
endif
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -V -u -U lock:w:$(FUSE_LOCK):m
else
	$(FLASH_BIN) -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(TARGET).elf
endif

format:
	@echo Checking code formatting...
	@$(FIND) . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\)' \
	-not -path "*gen*" \
	-not -name "*Pins.h" \
	-not -name "*Hardware.h" \
	-not -name "*MCU.h" \
	-not -name "*hal_conf*" \
	-exec $(CLANG_FORMAT) -style=file -i {} \;
ifeq ($(CF_FAIL_ON_DIFF), 1)
	git diff -s --exit-code
endif

clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR_BASE)/ ../Documentation/

#debugging
print-%:
	@echo '$($*)'