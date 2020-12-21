include Toolchain.mk

BUILD_DIR_BASE := ./build
TARGETNAME := mega2560
TYPE := app

ifneq (,$(wildcard $(BUILD_DIR_BASE)/TARGET))
    TARGETNAME := $(shell cat $(BUILD_DIR_BASE)/TARGET)
    DEBUG := $(shell cat $(BUILD_DIR_BASE)/DEBUG)
    TYPE := $(shell cat $(BUILD_DIR_BASE)/TYPE)
endif

ifeq (,$(wildcard ../targets/$(TARGETNAME).yml))
    $(error Target doesn't exist)
endif

ifeq ($(shell yq r ../targets/$(TARGETNAME).yml bootloader.use), true)
    HAS_BTLDR := 1
endif

ifeq (,$(findstring flashgen,$(MAKECMDGOALS)))
    ifeq ($(TYPE),boot)
        #verify if target has bootloader
        ifneq ($(HAS_BTLDR),1)
            $(error This target doesn't have bootloader)
        endif
    endif

    BUILD_DIR := $(BUILD_DIR_BASE)/$(TYPE)
else
    BUILD_DIR := $(BUILD_DIR_BASE)/flashgen
endif

BUILD_DIR := $(BUILD_DIR)/$(TARGETNAME)

ifeq ($(DEBUG),1)
    BUILD_TYPE := debug
else
    BUILD_TYPE := release
endif

BUILD_DIR := $(BUILD_DIR)/$(BUILD_TYPE)

TARGET := $(BUILD_DIR)/$(TARGETNAME)
.DEFAULT_GOAL := fw

#includes
#important - do not change the order of inclusion!
include Defines.mk
include Sources.mk

# if merged binary exists (bootloader+application), use it when running flash target
MERGED_TARGET := $(BUILD_DIR_BASE)/merged/$(TARGETNAME).hex

#when set to 1, format target will fail if there are any changes to the repository after formatting
CF_FAIL_ON_DIFF := 0

#passed to both c and c++ compiler
COMMON_FLAGS := \
-Wall \
-fdata-sections \
-ffunction-sections \
-fmessage-length=0 \
-fno-strict-aliasing \
-fshort-enums

#c++ compiler only
CPP_FLAGS := \
-std=c++11 \
-fno-rtti \
-fno-exceptions \
-fpermissive

#c compiler only
C_FLAGS := \
-std=c11 \
-Wno-parentheses-equality \
-Wno-unused-value

#assembler only
ASM_FLAGS := \
 -x assembler-with-cpp

#common linker flags
LDFLAGS := \
-Wl,--gc-sections \
-Wl,-Map="$(TARGET).map",--cref

#custom linker script
LDFLAGS += -T $(LINKER_FILE)

ifeq ($(ARCH),avr)
    TOOL_PREFIX := avr-
    SIZE_TOOL := avr-size -C --mcu=$(MCU)
    LDFLAGS += -Wl,--section-start=.text=$(FLASH_START_ADDR)

    CC := $(C_COMPILER_AVR)
    CXX := $(CPP_COMPILER_AVR)
    LINKER := $(LINKER_AVR)
    FLASH_BIN := $(FLASH_BIN_AVR)
    OPT := -Os

    C_FLAGS += \
    -fpack-struct

    COMMON_FLAGS += \
    -mrelax \
    -mmcu=$(MCU) \
    -funsigned-char \
    -funsigned-bitfields \
    -fno-jump-tables \
    -flto

    LDFLAGS += \
    -mrelax \
    -mmcu=$(MCU) \
    -flto
else ifeq ($(ARCH),stm32)
    TOOL_PREFIX := arm-none-eabi-
    SIZE_TOOL := arm-none-eabi-size
    CC := $(C_COMPILER_ARM)
    CXX := $(CPP_COMPILER_ARM)
    LINKER := $(LINKER_ARM)
    ASSEMBLER := $(C_COMPILER_ARM)
    FLASH_BIN := $(FLASH_BIN_ARM)
    OPT := -O2

    COMMON_FLAGS += \
    -mcpu=$(CPU) \
    -mthumb \
    -mno-unaligned-access \
    -target arm-none-eabi

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
    CC := $(C_COMPILER_x86)
    CXX := $(CPP_COMPILER_x86)
    LINKER := $(LINKER_x86)
    LDFLAGS :=
    SIZE_TOOL := size

    ifeq ($(DEBUG), 1)
        COMMON_FLAGS += -g -gdwarf-2
        OPT := -O0
    else
        OPT := -O3
    endif
endif

fw:
	@$(MAKE) --no-print-directory TYPE=$(TYPE) TARGETNAME=$(TARGETNAME) pre-build
	@$(MAKE) --no-print-directory TYPE=$(TYPE) TARGETNAME=$(TARGETNAME) $(TARGET).elf

$(TARGET).elf: $(OBJECTS) $(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release/generated_flash.bin_offset
	@echo Creating executable: $@
	@$(LINKER) -o$(TARGET).elf $(OBJECTS) $(LDFLAGS)
ifneq ($(ARCH),x86)
	@echo $(TARGETNAME) > $(BUILD_DIR_BASE)/TARGET
	@echo $(DEBUG) > $(BUILD_DIR_BASE)/DEBUG
	@echo $(TYPE) > $(BUILD_DIR_BASE)/TYPE
	@$(TOOL_PREFIX)objcopy --gap-fill 0xFF -O ihex $(TARGET).elf $(TARGET).hex
ifeq ($(TYPE),app)
#append metadata only for application
	@srec_cat $(TARGET).hex -Intel -exclude $(FW_METADATA_LOCATION) $$(($(FW_METADATA_LOCATION) + $(FW_METADATA_SIZE))) -MAximum_Little_Endian $(FW_METADATA_LOCATION) -o $(TARGET).hex -Intel
	@srec_cat $(TARGET).hex -Intel -Cyclic_Redundancy_Check_16_Little_Endian -MAximum-Address $(TARGET).hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $(TARGET).hex -Intel
#also append generated flash containing factory configuration
ifeq ($(ARCH),stm32)
	@srec_cat $(TARGET).hex -Intel $(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release/generated_flash.bin -binary -offset $(shell cat $(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release/generated_flash.bin_offset) -o $(TARGET).hex
	@$(TOOL_PREFIX)objcopy --gap-fill 0xFF -O ihex $(TARGET).hex $(TARGET).hex
endif
endif
	@$(TOOL_PREFIX)objcopy -I ihex "$(TARGET).hex" -O binary "$(TARGET).bin"
	@$(SIZE_TOOL) "$(TARGET).elf"
endif

sysexfw: $(TARGET).sysex

$(TARGET).sysex:
	@$(MAKE) --no-print-directory fw TARGETNAME=$(TARGETNAME) DEBUG=0 TYPE=app
	@echo Creating SysEx file...
	@../scripts/sysex_fw_create.sh $(TARGET).bin $(TARGET).sysex $(SYSEX_MANUFACTURER_ID_0) $(SYSEX_MANUFACTURER_ID_1) $(SYSEX_MANUFACTURER_ID_2) $(COMMAND_FW_UPDATE_START) $(COMMAND_FW_UPDATE_END) $(FW_UID)

merged:
	@mkdir -p $(BUILD_DIR_BASE)/merged
	@echo Building application...
	@$(MAKE) --no-print-directory TARGETNAME=$(TARGETNAME) TYPE=app
	@echo Building bootloader...
	@$(MAKE) --no-print-directory TARGETNAME=$(TARGETNAME) TYPE=boot
	@echo Combining application and bootloader...
ifeq ($(ARCH), avr)
	@srec_cat "$(BUILD_DIR_BASE)/app/$(TARGETNAME)/$(BUILD_TYPE)/$(TARGETNAME).hex" -Intel "$(BUILD_DIR_BASE)/boot/$(TARGETNAME)/$(BUILD_TYPE)/$(TARGETNAME).hex" -Intel -o $(MERGED_TARGET) -Intel
else
	@srec_cat "$(BUILD_DIR_BASE)/boot/$(TARGETNAME)/$(BUILD_TYPE)/$(TARGETNAME).hex" -Intel "$(BUILD_DIR_BASE)/app/$(TARGETNAME)/$(BUILD_TYPE)/$(TARGETNAME).hex" -Intel -o $(MERGED_TARGET) -Intel
endif
	@$(TOOL_PREFIX)objcopy -I ihex "$(MERGED_TARGET)" -O binary "$(MERGED_TARGET).bin"
	@echo Merged binary created: $(MERGED_TARGET)

pre-build: $(PINS_GEN_SOURCE) $(TSCREEN_GEN_SOURCE)

board/gen/$(TARGETNAME)/Pins.cpp: ../targets/$(TARGETNAME).yml
	@echo Generating pins...
	@../scripts/gen_pins.sh $<

application/io/touchscreen/design/$(TARGETNAME).cpp: application/io/touchscreen/design/$(TARGETNAME).json
	@echo Generating touchscreen coordinate file...
	@../scripts/gen_touchscreen.sh $<

$(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release/generated_flash.bin_offset:
ifeq ($(ARCH), stm32)
ifeq ($(TYPE),app)
	@$(MAKE) --no-print-directory TARGETNAME=$(TARGETNAME) TYPE=app DEBUG=0 flashgen
	@echo Generating factory flash configuration...
	@./$(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release/$(TARGETNAME).elf $(shell pwd)/$(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release/generated_flash.bin
endif
else
#just generate empty file if not needed to avoid re-linking on repeated calls of make on targets where this isn't used
	@mkdir -p $(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release
	@touch $(BUILD_DIR_BASE)/flashgen/$(TARGETNAME)/release/generated_flash.bin_offset
endif

flashgen: $(TARGET).elf

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@echo Building: $<
	@$(CC) $(COMMON_FLAGS) $(C_FLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@echo Building: $<
	@$(CXX) $(COMMON_FLAGS) $(CPP_FLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

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
ifneq (,$(wildcard $(MERGED_TARGET)))
	$(FLASH_BIN) -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(MERGED_TARGET)
else
	$(FLASH_BIN) -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(TARGET).hex
endif

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