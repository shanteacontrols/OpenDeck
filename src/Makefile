include Toolchain.mk

BUILD_DIR_BASE := ./build
TARGETNAME := mega2560

ifneq (,$(wildcard $(BUILD_DIR_BASE)/TARGET))
    TARGETNAME := $(shell cat $(BUILD_DIR_BASE)/TARGET)
    DEBUG := $(shell cat $(BUILD_DIR_BASE)/DEBUG)
    BOOT := $(shell cat $(BUILD_DIR_BASE)/BOOT)
endif

ifeq (,$(wildcard ../targets/$(TARGETNAME).yml))
    $(error Target doesn't exist)
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml bootloader.use), true)
    HAS_BTLDR := 1
endif

ifeq ($(BOOT),1)
    #verify if target has bootloader
    ifneq ($(HAS_BTLDR),1)
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
.DEFAULT_GOAL := fw

#includes
#important - do not change the order of inclusion!
include Defines.mk
include Sources.mk

# if merged binary exists (bootloader+application), use it when running flash target
MERGED_TARGET := $(BUILD_DIR_BASE)/merged/$(TARGETNAME).bin

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
    TOOL_PREFIX := avr-
    SIZE_TOOL := avr-size -C --mcu=$(MCU)
    LDFLAGS += -Wl,--section-start=.text=$(FLASH_START_ADDR)

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
    TOOL_PREFIX := arm-none-eabi-
    SIZE_TOOL := arm-none-eabi-size
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

    LDFLAGS += -Xlinker --defsym=FLASH_START=$(FLASH_START_ADDR)

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

fw:
	@$(MAKE) --no-print-directory BOOT=$(BOOT) TARGETNAME=$(TARGETNAME) pre-build
	@$(MAKE) --no-print-directory BOOT=$(BOOT) TARGETNAME=$(TARGETNAME) $(TARGET).elf

$(TARGET).elf: $(OBJECTS)
	@echo $(TARGETNAME) > $(BUILD_DIR_BASE)/TARGET
	@echo $(DEBUG) > $(BUILD_DIR_BASE)/DEBUG
	@echo $(BOOT) > $(BUILD_DIR_BASE)/BOOT
	@echo Creating executable: $@
	@$(CPP_COMPILER) -o$(TARGET).elf $(OBJECTS) $(LDFLAGS)
ifeq ($(ARCH), avr)
	@avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "$(TARGET).elf" "$(TARGET).hex"
endif
	@$(TOOL_PREFIX)objcopy --gap-fill 0xFF -O ihex $(TARGET).elf $(TARGET).hex
#append metadata only for application
ifneq ($(BOOT),1)
	@srec_cat $(TARGET).hex -Intel -exclude $(FW_METADATA_LOCATION) $$(($(FW_METADATA_LOCATION) + $(FW_METADATA_SIZE))) -MAximum_Little_Endian $(FW_METADATA_LOCATION) -o $(TARGET).hex -Intel
	@srec_cat $(TARGET).hex -Intel -Cyclic_Redundancy_Check_16_Little_Endian -MAximum-Address $(TARGET).hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $(TARGET).hex -Intel
endif
	@$(TOOL_PREFIX)objcopy -I ihex "$(TARGET).hex" -O binary "$(TARGET).bin"
	@$(SIZE_TOOL) "$(TARGET).elf"

sysexfw: $(TARGET).sysex

$(TARGET).sysex: $(TARGET).elf
ifeq ($(HAS_BTLDR),1)
ifneq ($(BOOT),1)
	@echo Creating SysEx file...
	@../scripts/sysex_fw_create.sh $(TARGET).bin $(TARGET).sysex $(SYSEX_MANUFACTURER_ID_0) $(SYSEX_MANUFACTURER_ID_1) $(SYSEX_MANUFACTURER_ID_2) $(COMMAND_FW_UPDATE_START) $(COMMAND_FW_UPDATE_END)
endif
endif

merged:
	@mkdir -p $(BUILD_DIR_BASE)/merged
	@echo Building application...
	@$(MAKE) --no-print-directory TARGETNAME=$(TARGETNAME) BOOT=0
	@echo Building bootloader...
	@$(MAKE) --no-print-directory TARGETNAME=$(TARGETNAME) BOOT=1
	@echo Combining application and bootloader...
ifeq ($(ARCH), avr)
	@avr-objcopy --pad-to=$(BOOT_START_ADDR) --gap-fill=0xFF -O binary "$(BUILD_DIR_BASE)/application/$(TARGETNAME)/release/$(TARGETNAME).elf" "$(BUILD_DIR_BASE)/application/$(TARGETNAME)/release/$(TARGETNAME)_padded.bin"
	@cat "$(BUILD_DIR_BASE)/application/$(TARGETNAME)/release/$(TARGETNAME)_padded.bin" "$(BUILD_DIR_BASE)/bootloader/$(TARGETNAME)/release/$(TARGETNAME).bin" > $(MERGED_TARGET)
else
	@arm-none-eabi-objcopy --pad-to=$(APP_START_ADDR) --gap-fill=0xFF -O binary "$(BUILD_DIR_BASE)/bootloader/$(TARGETNAME)/release/$(TARGETNAME).elf" "$(BUILD_DIR_BASE)/bootloader/$(TARGETNAME)/release/$(TARGETNAME)_padded.bin"
	@cat "$(BUILD_DIR_BASE)/bootloader/$(TARGETNAME)/release/$(TARGETNAME)_padded.bin" "$(BUILD_DIR_BASE)/application/$(TARGETNAME)/release/$(TARGETNAME).bin" > $(MERGED_TARGET)
endif
	@echo Merged binary created: $(MERGED_TARGET)

pre-build: $(GEN_SOURCES)

board/gen/$(TARGETNAME)/Pins.cpp: ../targets/$(TARGETNAME).yml
	@echo Generating pins...
	@../scripts/gen_pins.sh $<

application/io/touchscreen/coordinates/$(TARGETNAME).cpp: application/io/touchscreen/coordinates/$(TARGETNAME).json
	@echo Generating touchscreen coordinate file...
	@../scripts/gen_touchscreen.sh $<

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
ifneq (,$(wildcard $(MERGED_TARGET)))
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:w:$(MERGED_TARGET)
else
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:w:$(TARGET).hex
endif
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -V -u -U lock:w:$(FUSE_LOCK):m
else
ifneq (,$(wildcard $(MERGED_TARGET)))
	st-flash write $(MERGED_TARGET) $(BOOT_START_ADDR)
else
	st-flash write $(TARGET).bin $(BOOT_START_ADDR)
endif
endif

flash-bmp:
	$(FLASH_BIN) -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(TARGET).hex

ifeq ($(ARCH), avr)
flashread:
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:r:readcontents.bin:r
endif

format:
	@echo Checking code formatting...
	@$(FIND) . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\)' \
	-not -path "*gen*" \
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