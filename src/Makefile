include Toolchain.mk

BUILD_DIR_BASE  := ./build
TARGET          := discovery
TARGET_DEF_FILE := ../targets/$(TARGET).yml
TYPE            := app
SCRIPTS_DIR     := ../scripts

ifeq (,$(wildcard $(TARGET_DEF_FILE)))
    $(error Target doesn't exist)
endif

ifneq (,$(filter $(MAKECMDGOALS),cat-full cat-sysex sysexfw))
    BUILD_DIR := $(BUILD_DIR_BASE)/merged
else
    BUILD_DIR := $(BUILD_DIR_BASE)/$(TYPE)
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
    CC := $(C_COMPILER_NATIVE)
    CXX := $(CPP_COMPILER_NATIVE)
    LINKER := $(LINKER_NATIVE)
    LDFLAGS :=
    SIZE_TOOL := size

    ifeq ($(DEBUG), 1)
        COMMON_FLAGS += -g -gdwarf-2
        OPT := -O0
    else
        OPT := -O3
    endif
endif

binary: $(OUTPUT).elf

$(OUTPUT).elf: $(OBJECTS)
	@echo Creating executable: $@
	@$(LINKER) -o$(OUTPUT).elf $(OBJECTS) $(LDFLAGS)
ifeq (,$(findstring gen,$(TYPE)))
	@objcopy --gap-fill 0xFF -O ihex $(OUTPUT).elf $(OUTPUT).hex
endif
ifeq ($(TYPE),app)
#append metadata only for application
	@srec_cat $(OUTPUT).hex -Intel -exclude $(FW_METADATA_LOCATION) $$(($(FW_METADATA_LOCATION) + $(FW_METADATA_SIZE))) -MAximum_Little_Endian $(FW_METADATA_LOCATION) -o $(OUTPUT).hex -Intel
	@srec_cat $(OUTPUT).hex -Intel -Cyclic_Redundancy_Check_16_Little_Endian -MAximum-Address $(OUTPUT).hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $(OUTPUT).hex -Intel
endif
ifeq (,$(findstring gen,$(TYPE)))
	@objcopy -I ihex "$(OUTPUT).hex" -O binary "$(OUTPUT).bin"
endif
	@$(SIZE_TOOL) "$(OUTPUT).elf"
#indicate that new binary has been created
	@echo $(shell date +%s%N) > $(BUILD_TIME_FILE)

merged-full:
	@echo Building application...
	@$(MAKE) --no-print-directory TYPE=app TARGET=$(TARGET) DEBUG=$(DEBUG) pre-build
	@$(MAKE) --no-print-directory TYPE=app TARGET=$(TARGET) DEBUG=$(DEBUG) binary
	@echo Building bootloader...
	@$(MAKE) --no-print-directory TYPE=boot TARGET=$(TARGET) DEBUG=$(DEBUG) binary
	@echo Building sysexgen...
	@$(MAKE) --no-print-directory TYPE=sysexgen TARGET=$(TARGET) DEBUG=$(DEBUG) binary
ifeq ($(ARCH), stm32)
	@echo Building CDC...
	@$(MAKE) --no-print-directory TYPE=cdc TARGET=$(TARGET) DEBUG=$(DEBUG) binary
	@echo Building flashgen...
	@$(MAKE) --no-print-directory TYPE=flashgen TARGET=$(TARGET) DEBUG=$(DEBUG) binary
endif
#relink only if needed
	@NEW_BUILD_TIME=$$(cat $(BUILD_TIME_FILE)) && if [ $$(echo $$NEW_BUILD_TIME) = $$(echo $(LAST_BUILD_TIME)) ]; then \
	echo Merged binary up to date; \
	else \
	$(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) cat-full; \
	fi

#when building sysex binary, also run merged-full just in case so that full binary is re-linked if needed as well
merged-sysex: merged-full
#relink only if needed
	@NEW_BUILD_TIME=$$(cat $(BUILD_TIME_FILE)) && \
	if [ $$(echo $$NEW_BUILD_TIME) = $$(echo $(LAST_BUILD_TIME)) ] && [ -f $(BUILD_DIR)/$(TARGET).sysex ]; then \
	echo SysEx binary up to date; \
	else \
	$(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) cat-sysex; \
	fi

cat-full: $(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin
	@mkdir -p $(BUILD_DIR)
ifeq ($(ARCH), avr)
	@srec_cat \
	$(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/boot/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
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
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
endif
	@objcopy -I ihex "$(BUILD_DIR)/$(TARGET).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR)/$(TARGET).bin"
	@echo Merged binary created: $(BUILD_DIR)/$(TARGET).hex
#everytime new binary gets created, remove sysex binary
	@rm -f $(BUILD_DIR)/$(TARGET).sysex

cat-sysex:
ifeq ($(ARCH), avr)
	@srec_cat \
	$(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	-o $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
else
	@FACTORY_IMAGE_START_ADDR=$$(cat $(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin_offset) && \
	srec_cat \
	$(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/cdc/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin -binary -offset $$(echo $$FACTORY_IMAGE_START_ADDR) \
	-o $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
endif
	@objcopy -I ihex "$(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).bin"
	@rm $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
	@echo Creating SysEx file...
#raw syx file for use with amidi
	@$(BUILD_DIR_BASE)/sysexgen/$(TARGET)/$(BUILD_TYPE)/$(TARGET).elf $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).bin $(BUILD_DIR)/$(TARGET).sysex.syx
#ascii sysex file for use with web configurator
	@hexdump -v -e '/1 "%02x "' $(BUILD_DIR)/$(TARGET).sysex.syx | sed 's/f7/f7\n/g' | sed 's/^ *//' | tr a-z A-Z > $(BUILD_DIR)/$(TARGET).sysex
	@rm $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).bin
	@echo SysEx file created: $(BUILD_DIR)/$(TARGET).sysex

#just a shorter alias
sysexfw: merged-sysex

pre-build: $(PINS_GEN_SOURCE) $(TSCREEN_GEN_SOURCE)

board/gen/$(TARGET)/Pins.cpp: $(TARGET_DEF_FILE)
	@echo Generating board definitions...
	@$(SCRIPTS_DIR)/gen_board.sh $< $(SW_VERSION_MAJOR) $(SW_VERSION_MINOR) $(SW_VERSION_REVISION)

application/io/touchscreen/design/gen/$(TARGET).cpp: application/io/touchscreen/design/$(TARGET).json
	@echo Generating touchscreen coordinate file...
	@$(SCRIPTS_DIR)/gen_touchscreen.sh $<

ifeq ($(ARCH),stm32)
$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin: $(TARGET_DEF_FILE) $(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/application/database/Database.cpp.o $(BUILD_DIR_BASE)/app/$(TARGET)/$(BUILD_TYPE)/application/database/CustomInit.cpp.o
	@echo Generating factory flash configuration...
	@./$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/$(TARGET).elf $@
else
$(BUILD_DIR_BASE)/flashgen/$(TARGET)/$(BUILD_TYPE)/generated_flash.bin:
#just create dummy binary
	@mkdir -p $(@D)
	@touch $@
endif

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
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -C /etc/avrdude.conf -e -V -u -U lock:w:$(FUSE_UNLOCK):m -U efuse:w:$(FUSE_EXT):m -U hfuse:w:$(FUSE_HIGH):m -U lfuse:w:$(FUSE_LOW):m
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -C /etc/avrdude.conf -U flash:w:$(BUILD_DIR_BASE)/merged/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -C /etc/avrdude.conf -V -u -U lock:w:$(FUSE_LOCK):m
else
	st-flash write $(BUILD_DIR)/$(TARGET).hex $(BOOT_START_ADDR)
endif

flash-bmp:
	$(FLASH_BIN) -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(BUILD_DIR_BASE)/merged/$(TARGET)/$(BUILD_TYPE)/$(TARGET).hex

flash-dfu:
	@sudo dfu-util -d 0483:df11 -a 0 -i 0 -s 0x8000000:leave -D $(BUILD_DIR_BASE)/merged/$(TARGET)/$(BUILD_TYPE)/$(TARGET).bin

ifeq ($(ARCH), avr)
flashread:
	@$(FLASH_BIN) -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -C /etc/avrdude.conf -U flash:r:readcontents.bin:r
endif

format:
	@echo Checking code formatting...
	@$(FIND) . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\|c\)' \
	-not -path "**/gen/**/*" \
	-not -name "*hal_conf*" \
	-exec $(CLANG_FORMAT) -style=file -i {} +
ifeq ($(CF_FAIL_ON_DIFF), 1)
	git diff
	git diff -s --exit-code
endif

clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR_BASE)/ board/gen application/io/touchscreen/design/gen

#debugging
print-%:
	@echo '$($*)'