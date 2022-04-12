include Toolchain.mk

BUILD_DIR_BASE       := ./build
TARGET               := discovery
DEF_FILE_TARGET      := ../config/target/$(TARGET).yml
DEF_FILE_TSCREEN     := ../config/touchscreen/$(TARGET).json
TYPE                 := app
SCRIPTS_DIR          := ../scripts
DEPS_DIR             := deps
CLANG_TIDY_OUT       := $(BUILD_DIR_BASE)/clang-tidy-fixes.yaml

ifeq (,$(wildcard $(DEF_FILE_TARGET)))
    $(error Target doesn't exist)
endif

ifeq ($(DEBUG),1)
    BUILD_TYPE := debug
else
    BUILD_TYPE := release
endif

BUILD_DIR := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)

ifneq (,$(filter $(MAKECMDGOALS),concat))
    BUILD_DIR := $(BUILD_DIR)/merged
else
    BUILD_DIR := $(BUILD_DIR)/$(TYPE)
endif

OUTPUT           := $(BUILD_DIR)/$(TARGET)
BUILD_TIME_FILE  := $(BUILD_DIR_BASE)/lastbuild
LAST_BUILD_TIME  := $(shell cat $(BUILD_TIME_FILE) 2>/dev/null | awk '{print$$1}END{if(NR==0)print 0}')
FLASH_BINARY_DIR := $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged

SYSEX_BINARY_SUFFIX := _sysex

#includes
#important - do not change the order of inclusion!
include Defines.mk
include Sources.mk
-include flashing/$(FLASH_TOOL).mk

.DEFAULT_GOAL := merged

#when set to 1, format target will fail if there are any changes to the repository after formatting
CF_FAIL_ON_DIFF := 0

# when set to 1, lint target will fail if there are any changes to the repository after linting
CL_FAIL_ON_DIFF := 0

#passed to both c and c++ compiler
CPPFLAGS := \
-Wall \
-fdata-sections \
-ffunction-sections \
-fmessage-length=0 \
-fno-strict-aliasing \
-fshort-enums \
-Wno-unused-function

#c++ compiler only
CXXFLAGS := \
-std=c++17 \
-fno-rtti \
-fno-exceptions \
-fpermissive \
-fno-threadsafe-statics

#c compiler only
CFLAGS := \
-std=c11 \
-Wno-parentheses-equality \
-Wno-unused-value \
-Wno-implicit-function-declaration

#assembler only
ASFLAGS := \
-x assembler-with-cpp

#common linker flags
LDFLAGS := \
-Wl,--gc-sections \
-Wl,-Map="$(OUTPUT).map",--cref

#custom linker script
LDFLAGS += -T $(LINKER_FILE)
OPT := -Os

ifeq (,$(findstring gen,$(TYPE)))
    ifeq ($(ARCH),avr)
        CC         := $(CC_AVR)
        CXX        := $(CXX_AVR)
        LD         := $(LD_AVR)
        SIZE_TOOL  := avr-size -C --mcu=$(MCU)

        CFLAGS += \
        -fpack-struct

        CPPFLAGS += \
        -mrelax \
        -mmcu=$(MCU) \
        -funsigned-char \
        -funsigned-bitfields \
        -fno-jump-tables

        ifeq ($(MCU), atmega16u2)
            CPPFLAGS += -flto
        endif

        LDFLAGS += \
        -Wl,--section-start=.text=$(FLASH_START_ADDR) \
        -mrelax \
        -mmcu=$(MCU)

        ifeq ($(DEBUG), 1)
            CPPFLAGS += -g -gdwarf-2
            LDFLAGS  += -g -gdwarf-2
            OPT      := -Og
        endif

        ifeq ($(MCU), atmega16u2)
            LDFLAGS += -flto
        endif
    else ifeq ($(ARCH),arm)
        CC         := $(CC_ARM)
        CXX        := $(CXX_ARM)
        LD         := $(LD_ARM)
        AS         := $(CC_ARM)
        SIZE_TOOL  := size

        CPPFLAGS += \
        -mcpu=$(CPU) \
        -mthumb \
        -mno-unaligned-access

        ifneq (,$(findstring clang,$(CC_ARM)))
            CPPFLAGS += -target arm-none-eabi
        endif

        LDFLAGS += \
        -mcpu=$(CPU) \
        -mthumb \
        -specs=nano.specs \
        -specs=nosys.specs

        LDFLAGS += -Xlinker --defsym=CORE_FLASH_START_ADDR=$(FLASH_START_ADDR)

        ifeq ($(DEBUG), 1)
            CPPFLAGS += -g -gdwarf-2
            LDFLAGS  += -g -gdwarf-2
            OPT      := -Og
        endif

        ifneq ($(FPU),)
            CPPFLAGS += -mfpu=$(FPU)
            LDFLAGS  += -mfpu=$(FPU)
        endif

        ifneq ($(FLOAT-ABI),)
            CPPFLAGS += -mfloat-abi=$(FLOAT-ABI)
            LDFLAGS  += -mfloat-abi=$(FLOAT-ABI)
        endif
    endif
else
    CC        := $(CC_NATIVE)
    CXX       := $(CXX_NATIVE)
    LD        := $(LD_NATIVE)
    SIZE_TOOL := size

    LDFLAGS :=

    ifeq ($(DEBUG), 1)
        CPPFLAGS += -g -gdwarf-2
        OPT      := -O0
    else
        OPT := -O3
    endif
endif

binary: $(OUTPUT).elf

$(OUTPUT).elf: $(OBJECTS)
#perform the linking only if sources have been defined
ifneq ($(OBJECTS),)
	@echo Creating executable: $@
	@$(LD) -o$(OUTPUT).elf $(OBJECTS) $(LDFLAGS)
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
endif

merged:
	@echo Starting build for $(TARGET)
	@$(MAKE) --no-print-directory TYPE=app TARGET=$(TARGET) DEBUG=$(DEBUG) generate
	@$(MAKE) --no-print-directory TYPE=app TARGET=$(TARGET) DEBUG=$(DEBUG) pre-build
	@echo Building application...
	@$(MAKE) --no-print-directory TYPE=app TARGET=$(TARGET) DEBUG=$(DEBUG) binary
	@echo Building bootloader...
	@$(MAKE) --no-print-directory TYPE=boot TARGET=$(TARGET) DEBUG=$(DEBUG) binary
	@echo Building sysexgen...
	@$(MAKE) --no-print-directory TYPE=sysexgen TARGET=$(TARGET) DEBUG=$(DEBUG) binary
	@echo Building flashgen...
	@$(MAKE) --no-print-directory TYPE=flashgen TARGET=$(TARGET) DEBUG=$(DEBUG) binary
#relink only if needed
	@NEW_BUILD_TIME=$$(cat $(BUILD_TIME_FILE)) && if [ $$(echo $$NEW_BUILD_TIME) = $$(echo $(LAST_BUILD_TIME)) ]; then \
	echo Merged binary up to date; \
	else \
	$(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) concat; \
	$(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) post-build; \
	fi

concat: $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.bin
	@mkdir -p $(BUILD_DIR)
ifeq ($(ARCH), avr)
	@srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/app/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/boot/$(TARGET).hex -Intel \
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
	@srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/app/$(TARGET).hex -Intel \
	-o $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
else
#define FACTORY_IMAGE_START_ADDR variable as shell variable instead of make variable:
#when this recipe is started, generated_flash.bin_offset doesn't exist yet, so make will
#complain that it cannot find it if it were defined like this: $(shell cat $(BUILD_DIR_BASE)/flashgen/$(TARGET)/release/generated_flash.bin_offset)
	@FACTORY_IMAGE_START_ADDR=$$(cat $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.bin_offset) && \
	srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/boot/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/app/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.bin -binary -offset $$(echo $$FACTORY_IMAGE_START_ADDR) \
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
	@FACTORY_IMAGE_START_ADDR=$$(cat $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.bin_offset) && \
	srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/app/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.bin -binary -offset $$(echo $$FACTORY_IMAGE_START_ADDR) \
	-o $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
endif
	@objcopy -I ihex "$(BUILD_DIR)/$(TARGET).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR)/$(TARGET).bin"
	@objcopy -I ihex "$(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).bin"
	@rm $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
	@echo Merged binary created: $(BUILD_DIR)/$(TARGET).hex
	@echo Creating SysEx file...
#raw syx file for use with amidi
	@$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/sysexgen/$(TARGET).elf $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).bin $(BUILD_DIR)/$(TARGET).sysex.syx
#ascii sysex file for use with web configurator
	@hexdump -v -e '/1 "%02x "' $(BUILD_DIR)/$(TARGET).sysex.syx | sed 's/f7/f7\n/g' | sed 's/^ *//' | tr a-z A-Z > $(BUILD_DIR)/$(TARGET).sysex
	@echo SysEx file created: $(BUILD_DIR)/$(TARGET).sysex

generate: $(GEN_FILES)
pre-build::
post-build::

$(TARGET_GEN_HEADER): $(DEF_FILE_TARGET)
	@$(SCRIPTS_DIR)/gen/gen_target.sh $(DEF_FILE_TARGET) $(BOARD_GEN_DIR_TARGET)

$(TSCREEN_GEN_SOURCE): $(DEF_FILE_TSCREEN)
	@$(SCRIPTS_DIR)/gen/gen_touchscreen.sh $< $@

ifeq ($(ARCH),arm)
$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.bin: $(DEF_FILE_TARGET) $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/app/application/database/Database.cpp.o $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/app/application/database/CustomInit.cpp.o
	@$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/$(TARGET).elf $@
else
$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.bin:
#just create dummy binary
	@mkdir -p $(@D)
	@touch $@
endif

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@echo Building: $<
	@ccache $(CC) $(CPPFLAGS) $(CFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@echo Building: $<
	@ccache $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

define COMPILE_ASM
	@mkdir -p $(@D)
	@echo Building: $<
	@ccache $(AS) $(CPPFLAGS) $(CFLAGS) $(ASFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"
endef

$(BUILD_DIR)/%.s.o: %.s
	$(COMPILE_ASM)

$(BUILD_DIR)/%.S.o: %.S
	$(COMPILE_ASM)

ifeq ($(ARCH), avr)
flashread:
	@avrdude -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -C /etc/avrdude.conf -U flash:r:readcontents.bin:r
endif

format:
	@echo Checking code formatting...
	@$(FIND) . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\|c\)' \
	-not -path "**/gen/**/*" \
	-not -path "**/CMSIS/*" \
	-not -name "*hal_conf*" \
	-exec $(CLANG_FORMAT) -style=file -i {} +
ifeq ($(CF_FAIL_ON_DIFF), 1)
	git diff
	git diff -s --exit-code
endif

lint:
	@$(SCRIPTS_DIR)/tidy.sh \
	--mcu=$(MCU) \
	--output=$(CLANG_TIDY_OUT) \
	--compiler=$(CC) \
	--option-files-dir=..
ifeq ($(CL_FAIL_ON_DIFF), 1)
	@if [ -s $(CLANG_TIDY_OUT) ]; then \
		echo Lint issues found:; \
		cat $(CLANG_TIDY_OUT); \
		false; \
	fi
endif

clean:
	@echo Cleaning up.
	@rm -rf $(BUILD_DIR_BASE) $(BOARD_GEN_DIR_BASE) $(APP_GEN_DIR) compile_commands.json

#debugging
print-%:
	@echo '$($*)'