# Important - do not change the order of inclusion!
include Toolchain.mk
include Setup.mk
-include firmware/board/Makefile
-include firmware/$(TYPE)/Makefile
-include tools/$(TYPE)/Makefile
include Defines.mk
include Sources.mk
-include flashing/$(FLASH_TOOL).mk

.DEFAULT_GOAL := all

binary: $(OUTPUT).elf

$(OUTPUT).elf: $(OBJECTS)
# Perform the linking only if sources have been defined
ifneq ($(OBJECTS),)
	$(Q)echo Creating executable: $@
	$(Q)$(LD) -o$(OUTPUT).elf $(OBJECTS) $(LDFLAGS) $(LDLIBS)
ifeq (,$(findstring gen,$(TYPE)))
	$(Q)objcopy --gap-fill 0xFF -O ihex $(OUTPUT).elf $(OUTPUT).hex
ifeq ($(TYPE),application)
# Append metadata only for application if bootloader is supported
ifneq (,$(findstring PROJECT_MCU_SUPPORT_BOOTLOADER,$(DEFINES)))
	$(Q)srec_cat $(OUTPUT).hex -Intel -exclude $(PROJECT_MCU_FLASH_ADDR_FW_METADATA_START) $(PROJECT_MCU_FLASH_ADDR_FW_METADATA_END) -MAximum_Little_Endian $(PROJECT_MCU_FLASH_ADDR_FW_METADATA_START) -o $(OUTPUT).hex -Intel
	$(Q)srec_cat $(OUTPUT).hex -Intel -Cyclic_Redundancy_Check_16_Little_Endian -MAximum-Address $(OUTPUT).hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $(OUTPUT).hex -Intel
endif
endif
	$(Q)objcopy -I ihex "$(OUTPUT).hex" -O binary "$(OUTPUT).bin"
endif
ifneq (,$(SIZE_TOOL))
	$(Q)$(SIZE_TOOL) "$(OUTPUT).elf"
endif
# Indicate that new binary has been created
	$(Q)echo $(shell date +%s%N) > $(BUILD_TIME_FILE)
endif

all:
	$(Q)echo Starting build for $(TARGET)
	$(Q)$(MAKE) --no-print-directory TYPE=application TARGET=$(TARGET) DEBUG=$(DEBUG) generate V=$(V)
	$(Q)$(MAKE) --no-print-directory TYPE=application TARGET=$(TARGET) DEBUG=$(DEBUG) pre-build V=$(V)
	$(Q)echo Building application...
	$(Q)$(MAKE) --no-print-directory TYPE=application TARGET=$(TARGET) DEBUG=$(DEBUG) binary V=$(V)
	$(Q)echo Building bootloader...
	$(Q)$(MAKE) --no-print-directory TYPE=bootloader TARGET=$(TARGET) DEBUG=$(DEBUG) binary V=$(V)
	$(Q)echo Building sysexgen...
	$(Q)$(MAKE) --no-print-directory TYPE=sysexgen TARGET=$(TARGET) DEBUG=$(DEBUG) binary V=$(V)
	$(Q)echo Building flashgen...
	$(Q)$(MAKE) --no-print-directory TYPE=flashgen TARGET=$(TARGET) DEBUG=$(DEBUG) binary V=$(V)
# Relink only if needed
	$(Q)NEW_BUILD_TIME=$$(cat $(BUILD_TIME_FILE)) && if [ $$(echo $$NEW_BUILD_TIME) = $$(echo $(LAST_BUILD_TIME)) ]; then \
	echo Merged binary up to date; \
	else \
	$(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) concat V=$(V); \
	$(MAKE) --no-print-directory TARGET=$(TARGET) DEBUG=$(DEBUG) post-build V=$(V); \
	fi

concat: $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.hex
	$(Q)mkdir -p $(BUILD_DIR)
# If PROJECT_MCU_DATABASE_INIT_DATA is set to 1, DB data will be initialized in firmware.
# Otherwise, initial DB data is read from generated binary file appended
# to application binary.
ifneq (,$(findstring PROJECT_MCU_DATABASE_INIT_DATA=1,$(DEFINES)))
ifneq (,$(findstring PROJECT_MCU_SUPPORT_BOOTLOADER,$(DEFINES)))
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/bootloader/$(TARGET).hex -Intel \
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
else
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/$(TARGET).hex -Intel \
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
endif
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/$(TARGET).hex -Intel \
	-o $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
else
ifneq (,$(findstring PROJECT_MCU_SUPPORT_BOOTLOADER,$(DEFINES)))
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/bootloader/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.hex -Intel \
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
else
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.hex -Intel \
	-o $(BUILD_DIR)/$(TARGET).hex -Intel
endif
	$(Q)srec_cat \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/$(TARGET).hex -Intel \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.hex -Intel \
	-o $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
endif
	$(Q)objcopy -I ihex "$(BUILD_DIR)/$(TARGET).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR)/$(TARGET).bin"
	$(Q)objcopy -I ihex "$(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex" --gap-fill 0xFF -O binary "$(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).bin"
	$(Q)rm $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).hex
	$(Q)echo Merged binary created: $(BUILD_DIR)/$(TARGET).hex
ifneq (,$(findstring PROJECT_MCU_SUPPORT_BOOTLOADER,$(DEFINES)))
	$(Q)echo Creating SysEx file...
# Raw syx file for use with amidi
	$(Q)$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/sysexgen/$(TARGET).elf $(BUILD_DIR)/$(TARGET)$(SYSEX_BINARY_SUFFIX).bin $(BUILD_DIR)/$(TARGET).sysex.syx
# ASCII sysex file for use with web configurator
	$(Q)hexdump -v -e '/1 "%02x "' $(BUILD_DIR)/$(TARGET).sysex.syx | sed 's/f7/f7\n/g' | sed 's/^ *//' | tr a-z A-Z > $(BUILD_DIR)/$(TARGET).sysex
	$(Q)echo SysEx file created: $(BUILD_DIR)/$(TARGET).sysex
endif

generate: $(GEN_FILES)
pre-build::

post-build::
ifeq ($(USE_UF2),1)
	@../modules/core/modules/tinyusb/tools/uf2/utils/uf2conv.py \
	-b $(CORE_MCU_FLASH_START_ADDR) \
	-f $(UF2_FAMILY_ID) \
	-o $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).uf2 \
	-c \
	$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/merged/$(TARGET).bin
endif

$(TARGET_GEN_HEADER):
	$(Q)$(SCRIPTS_DIR)/gen/gen_target.sh \
	--project=$(PROJECT) \
	--target-config=$(DEF_FILE_TARGET) \
	--gen-dir-target=$(TARGET_GEN_DIR) \
	--base-gen-dir-mcu=$(MCU_GEN_DIR_BASE)

$(TSCREEN_GEN_SOURCE):
	$(Q)$(SCRIPTS_DIR)/gen/gen_touchscreen.sh \
	--config=$(DEF_FILE_TSCREEN) \
	--gen-dir=$(APPLICATION_GEN_DIR_TARGET) \
	--img-dir=$(ROOT_MAKEFILE_DIR)/../bin/touchscreen/img/$(TARGET)

ifeq ($(CORE_MCU_ARCH),arm)
$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.hex: $(DEF_FILE_TARGET) $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/firmware/application/database/Database.cpp.o $(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/application/firmware/application/database/CustomInit.cpp.o
	$(Q)$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/$(TARGET).elf $@
else
$(BUILD_DIR_BASE)/$(TARGET)/$(BUILD_TYPE)/flashgen/generated_flash.hex:
# Just create dummy binary
	$(Q)mkdir -p $(@D)
	$(Q)touch $@
endif

$(BUILD_DIR)/%.c.o: %.c
	$(Q)mkdir -p $(@D)
	$(Q)echo Building: $<
	$(Q)$(CCACHE) $(CC) $(CPPFLAGS) $(CFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(Q)mkdir -p $(@D)
	$(Q)echo Building: $<
	$(Q)$(CCACHE) $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

define COMPILE_ASM
	$(Q)mkdir -p $(@D)
	$(Q)echo Building: $<
	$(Q)$(CCACHE) $(AS) $(CPPFLAGS) $(CFLAGS) $(ASFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"
endef

$(BUILD_DIR)/%.s.o: %.s
	$(COMPILE_ASM)

$(BUILD_DIR)/%.S.o: %.S
	$(COMPILE_ASM)

format:
	@echo Checking code formatting...
	@$(FIND) . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\|c\)' \
	-not -path "**/$(DEPS_SUBDIR_BASE)/**/*" \
	-not -path "**/$(GENERATED_SUBDIR_BASE)/**/*" \
	-exec $(CLANG_FORMAT) -style=file -i {} +
ifeq ($(CF_FAIL_ON_DIFF), 1)
	git diff
	git diff -s --exit-code
endif

lint:
	@$(SCRIPTS_DIR)/clang_tidy_wrapper.sh \
	--mcu=$(CORE_MCU_MODEL) \
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
	@rm -rf \
	$(BUILD_DIR_BASE) \
	$(MCU_GEN_DIR_BASE) \
	$(TARGET_GEN_DIR_BASE) \
	$(APPLICATION_GEN_DIR_BASE) \
	compile_commands.json

# Debugging
print-%:
	@echo '$($*)'