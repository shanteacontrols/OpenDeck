include Toolchain.mk

BUILD_DIR_BASE := ./build
TARGET := discovery
TYPE := app

ifeq (,$(wildcard ../targets/$(TARGET).yml))
    $(error Target doesn't exist)
endif

ifeq (,$(findstring flashgen,$(MAKECMDGOALS)))
    BUILD_DIR := $(BUILD_DIR_BASE)/$(TYPE)
else
    BUILD_DIR := $(BUILD_DIR_BASE)/flashgen
endif

BUILD_DIR := $(BUILD_DIR)/$(TARGET)

ifeq ($(DEBUG),1)
    BUILD_TYPE := debug
else
    BUILD_TYPE := release
endif

BUILD_DIR       := $(BUILD_DIR)/$(BUILD_TYPE)
OUTPUT          := $(BUILD_DIR)/$(TARGET)
BUILD_TIME_FILE := $(BUILD_DIR_BASE)/lastbuild
LAST_BUILD_TIME := $(shell cat $(BUILD_TIME_FILE) 2>/dev/null | awk '{print$$1}END{if(NR==0)print 0}')

SYSEX_BINARY_SUFFIX := _sysex

ifneq (,$(findstring sysex,$(MAKECMDGOALS)))
    SYSEX_MERGE := 1
endif

#includes
#important - do not change the order of inclusion!
include Defines.mk
include Sources.mk

MERGED_TARGET := $(BUILD_DIR_BASE)/merged/$(TARGET)
.DEFAULT_GOAL := merged-full

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
-Wl,-Map="$(OUTPUT).map",--cref

#custom linker script
LDFLAGS += -T $(LINKER_FILE)
OPT := -Os

ifeq ($(ARCH),avr)
    SIZE_TOOL := avr-size -C --mcu=$(MCU)
    LDFLAGS += -Wl,--section-start=.text=$(FLASH_START_ADDR)

    CC := $(C_COMPILER_AVR)
    CXX := $(CPP_COMPILER_AVR)
    LINKER := $(LINKER_AVR)
    FLASH_BIN := $(FLASH_BIN_AVR)

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
    SIZE_TOOL := size
    CC := $(C_COMPILER_ARM)
    CXX := $(CPP_COMPILER_ARM)
    LINKER := $(LINKER_ARM)
    ASSEMBLER := $(C_COMPILER_ARM)
    FLASH_BIN := $(FLASH_BIN_ARM)

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

fw: $(OUTPUT).elf

$(OUTPUT).elf: $(OBJECTS) $(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin_offset
	@echo Creating executable: $@
	@$(LINKER) -o$(OUTPUT).elf $(OBJECTS) $(LDFLAGS)
ifneq ($(ARCH),x86)
	@objcopy --gap-fill 0xFF -O ihex $(OUTPUT).elf $(OUTPUT).hex
ifeq ($(TYPE),app)
#append metadata only for application
	@srec_cat $(OUTPUT).hex -Intel -exclude $(FW_METADATA_LOCATION) $$(($(FW_METADATA_LOCATION) + $(FW_METADATA_SIZE))) -MAximum_Little_Endian $(FW_METADATA_LOCATION) -o $(OUTPUT).hex -Intel
	@srec_cat $(OUTPUT).hex -Intel -Cyclic_Redundancy_Check_16_Little_Endian -MAximum-Address $(OUTPUT).hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $(OUTPUT).hex -Intel
endif
	@objcopy -I ihex "$(OUTPUT).hex" -O binary "$(OUTPUT).bin"
	@$(SIZE_TOOL) "$(OUTPUT).elf"
endif
#indicate that new binary has been created
	@echo $(shell date +%s%N) > $(BUILD_TIME_FILE)

merged-full:
	@mkdir -p $(MERGED_TARGET)
	@echo Building application...
	@$(MAKE) --no-print-directory TYPE=app TARGET=$(TARGET) DEBUG=$(DEBUG) pre-build
	@$(MAKE) --no-print-directory TYPE=app TARGET=$(TARGET) DEBUG=$(DEBUG) fw
	@echo Building bootloader...
	@$(MAKE) --no-print-directory TYPE=boot TARGET=$(TARGET) DEBUG=$(DEBUG) fw
ifeq ($(ARCH), stm32)
	@echo Building CDC...
	@$(MAKE) --no-print-directory TYPE=cdc TARGET=$(TARGET) DEBUG=$(DEBUG) fw
endif
#relink only if needed
	@NEW_BUILD_TIME=$$(cat $(BUILD_TIME_FILE)) && if [ $$(echo $$NEW_BUILD_TIME) = $$(echo $(LAST_BUILD_TIME)) ]; then \
	echo Merged binary up to date; \
	else \
	$(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) merge; \
	fi

merge:
ifeq ($(ARCH), avr)
	@srec_cat \
	$(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/boot/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	-o $(MERGED_TARGET).hex -Intel && \
	srec_cat \
	$(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	-o $(MERGED_TARGET)$(SYSEX_BINARY_SUFFIX).hex
else
#define FACTORY_IMAGE_START_ADDR variable as shell variable instead of make variable:
#when this recipe is started, generated_flash.bin_offset doesn't exist yet, so make will
#complain that it cannot find it if it were defined like this: $(shell cat $(BUILD_DIR_BASE)/flashgen/$(TARGET)/release/generated_flash.bin_offset)
	@FACTORY_IMAGE_START_ADDR=$$(cat $(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin_offset) && \
	srec_cat \
	$(BUILD_DIR_BASE)/boot/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/cdc/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin -binary -offset $$(echo $$FACTORY_IMAGE_START_ADDR) \
	-o $(MERGED_TARGET).hex -Intel && \
	srec_cat \
	$(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/cdc/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin -binary -offset $$(echo $$FACTORY_IMAGE_START_ADDR) \
	-o $(MERGED_TARGET)$(SYSEX_BINARY_SUFFIX).hex
endif
	@objcopy -I ihex "$(MERGED_TARGET).hex" --gap-fill 0xFF -O binary "$(MERGED_TARGET).bin"
	@objcopy -I ihex "$(MERGED_TARGET)$(SYSEX_BINARY_SUFFIX).hex" --gap-fill 0xFF -O binary "$(MERGED_TARGET)$(SYSEX_BINARY_SUFFIX).bin"
	@echo Merged binary created: $(MERGED_TARGET).hex
#everytime new binary gets created, remove sysex binary
	@rm -f $(MERGED_TARGET).sysex

#when building sysex binary, also run merged-full just in case so that full binary is re-linked if needed as well
merged-sysex: merged-full
#relink only if needed
	@NEW_BUILD_TIME=$$(cat $(BUILD_TIME_FILE)) && \
	if [ $$(echo $$NEW_BUILD_TIME) = $$(echo $(LAST_BUILD_TIME)) ] && [ -f $(MERGED_TARGET).sysex ]; then \
	echo SysEx binary up to date; \
	else \
	echo Creating SysEx file...; \
	../scripts/sysex_fw_create.sh $(MERGED_TARGET)$(SYSEX_BINARY_SUFFIX).bin $(MERGED_TARGET).sysex $(SYSEX_MANUFACTURER_ID_0) $(SYSEX_MANUFACTURER_ID_1) $(SYSEX_MANUFACTURER_ID_2) $(COMMAND_FW_UPDATE_START) $(COMMAND_FW_UPDATE_END) $(FW_UID); \
	echo SysEx file created: $(MERGED_TARGET).sysex; \
	fi

#just a shorter alias
sysexfw: merged-sysex

pre-build: $(PINS_GEN_SOURCE) $(TSCREEN_GEN_SOURCE)

board/gen/$(TARGET)/Pins.cpp: ../targets/$(TARGET).yml
	@echo Generating pins...
	@../scripts/gen_pins.sh $<

application/io/touchscreen/design/$(TARGET).cpp: application/io/touchscreen/design/$(TARGET).json
	@echo Generating touchscreen coordinate file...
	@../scripts/gen_touchscreen.sh $<

$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin_offset:
ifeq ($(ARCH), stm32)
ifeq ($(TYPE),app)
	@$(MAKE) --no-print-directory TARGET=$(TARGET) TYPE=app DEBUG=$(DEBUG) flashgen
	@echo Generating factory flash configuration...
	@./$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/$(TARGET).elf $(shell pwd)/$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin
endif
else
#just generate empty file if not needed to avoid re-linking on repeated calls of make on targets where this isn't used
	@mkdir -p $(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)
	@touch $(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin_offset
endif

flashgen: $(OUTPUT).elf

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
ifneq (,$(wildcard $(MERGED_TARGET).hex))
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:w:$(MERGED_TARGET).hex
else
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:w:$(OUTPUT).hex
endif
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -V -u -U lock:w:$(FUSE_LOCK):m
else
ifneq (,$(wildcard $(MERGED_TARGET).hex))
	st-flash write $(MERGED_TARGET).hex $(BOOT_START_ADDR)
else
	st-flash write $(OUTPUT).bin $(BOOT_START_ADDR)
endif
endif

flash-bmp:
ifneq (,$(wildcard $(MERGED_TARGET).hex))
	$(FLASH_BIN) -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(MERGED_TARGET).hex
else
	$(FLASH_BIN) -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(OUTPUT).hex
endif

ifeq ($(ARCH), avr)
flashread:
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:r:readcontents.bin:r
endif

format:
	@echo Checking code formatting...
	@$(FIND) . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\|c\)' \
	-not -path "**/gen/**/*" \
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