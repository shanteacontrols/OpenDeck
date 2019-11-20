BUILD_DIR_BASE := ./build/

TARGETNAME := fw_opendeck

ifneq (,$(wildcard $(BUILD_DIR_BASE)/TARGET))
    TARGETNAME := $(shell cat $(BUILD_DIR_BASE)/TARGET)
    DEBUG := $(shell cat $(BUILD_DIR_BASE)/DEBUG)
endif

ifneq ($(findstring $(TARGETNAME), $(shell cat targets.txt)), $(TARGETNAME))
    $(error Target doesn't exist)
endif

ifeq ($(DEBUG),1)
    BUILD_DIR := $(BUILD_DIR_BASE)/$(TARGETNAME)/$(VARIANT)/debug
else
    BUILD_DIR := $(BUILD_DIR_BASE)/$(TARGETNAME)/$(VARIANT)/release
endif

TARGET := $(BUILD_DIR)/$(TARGETNAME)
.DEFAULT_GOAL := $(TARGET).elf

#includes
#important - do not change the order of inclusion!
include Defines.mk
include Sources.mk

CLANG_FORMAT := clang-format

#when set to 1, format target will fail if there are any changes to the repository after formatting
CF_FAIL_ON_DIFF := 0

#common compiler flags
COMMON_FLAGS := \
-Wall \
-fdata-sections \
-ffunction-sections \
-fmessage-length=0 \
-fno-strict-aliasing

#c only
CFLAGS := \
-std=c11

#cpp only
CPPFLAGS := \
-std=c++11 \
-fno-rtti \
-fno-exceptions \
-fpermissive

#common linker flags
LDFLAGS := \
-Wl,--gc-sections \
-Wl,-Map="$(TARGET).map",--cref

ifeq ($(ARCH),avr)
    ifneq ($(filter boot_%, $(TARGETNAME)), )
        #make sure to link .text at correct address in bootloader
        LDFLAGS += -Wl,--section-start=.text=$(BOOT_START_ADDR)
    endif

    #use link time optimisations only on 32u4 MCU due to the heavy memory constrains
    ifeq ($(MCU), atmega32u4)
        COMMON_FLAGS += -flto
        LDFLAGS += -flto
    endif

    #append length only in firmware
    ifeq ($(findstring fw,$(TARGETNAME)), fw)
        LEN_APPEND := 1
    endif

    CPP := avr-gcc
    CXX := avr-g++
    OPT := -Os

    CFLAGS += \
    -fpack-struct

    COMMON_FLAGS += \
    -mrelax \
    -mmcu=$(MCU) \
    -funsigned-char \
    -funsigned-bitfields \
    -fshort-enums \
    -fno-jump-tables

    LDFLAGS += \
    -mrelax \
    -mmcu=$(MCU)
else ifeq ($(ARCH),stm32)
    CPP := arm-none-eabi-gcc
    CXX := arm-none-eabi-g++
    ASM := arm-none-eabi-gcc -x assembler-with-cpp
    OPT := -O2

    COMMON_FLAGS += \
    -mcpu=$(CPU) \
    -mthumb \
    -mfpu=$(FPU) \
    -mfloat-abi=$(FLOAT-ABI)

    LDFLAGS += \
    -mcpu=$(CPU) \
    -mthumb \
    -mfpu=$(FPU) \
    -mfloat-abi=$(FLOAT-ABI) \
    -specs=nano.specs \
    -specs=nosys.specs

    ifeq ($(DEBUG), 1)
        COMMON_FLAGS += -g -gdwarf-2
        LDFLAGS += -g -gdwarf-2
        OPT := -Og
    endif
else
    $(error Unsupported architecture)
endif

#custom linker script
LDFLAGS += -T board/$(ARCH)/variants/$(MCU)/$(MCU).ld

#use windows binary on wsl since HID access isn't possible in wsl
ifeq ($(findstring Microsoft,$(shell uname -r)), Microsoft)
    FLASH_BIN := @cmd.exe /c "..\bin\dfu\hid_bootloader_loader_win.exe"
else ifeq ($(shell uname), Linux)
    FLASH_BIN := @ ../bin/dfu/hid_bootloader_loader_linux
else ifeq ($(shell uname), Darwin)
    FLASH_BIN := @ ../bin/dfu/hid_bootloader_loader_mac
else
    $(error Unsupported platform)
endif

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CPP) $(COMMON_FLAGS) $(CFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"
	@echo Finished building: $<

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(COMMON_FLAGS) $(CPPFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"
	@echo Finished building: $<

$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(@D)
	@$(ASM) $(COMMON_FLAGS) $(CFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"
	@echo Finished building: $<

$(TARGET).elf: $(OBJECTS)
	@echo $(TARGETNAME) > $(BUILD_DIR_BASE)/TARGET
	@echo $(DEBUG) > $(BUILD_DIR_BASE)/DEBUG
	@#link
	@$(CXX) -o$(TARGET).elf $(OBJECTS) $(LDFLAGS)
	@echo Finished building target: $@
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
	@arm-none-eabi-size "$(TARGET).elf"
endif

flash:
ifeq ($(ARCH), avr)
	@avrdude -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -e -V -u -U lock:w:$(FUSE_UNLOCK):m -U efuse:w:$(FUSE_EXT):m -U hfuse:w:$(FUSE_HIGH):m -U lfuse:w:$(FUSE_LOW):m
	@avrdude -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -U flash:w:$(TARGET).hex
	@avrdude -p $(MCU) -P /dev/$(PORT) -b 19200 -c avrisp -V -u -U lock:w:$(FUSE_LOCK):m
else
	arm-none-eabi-gdb -nx --batch \
	-ex 'target extended-remote /dev/$(PORT)' \
	-ex 'monitor swdp_scan' \
	-ex 'attach 1' \
	-ex 'load' \
	-ex 'compare-sections' \
	-ex 'kill' \
	$(TARGET).elf
endif

#firmware update with bootloader
flashboot:
	$(FLASH_BIN) $(MCU) build/$(TARGETNAME).hex

#lists all serial ports - used to find out where the programmer is connected
ser_list:
	dmesg | grep tty

#other targets
clean:
	@echo Cleaning up.
	@rm -rf build/ ../Documentation/

exe-scripts:
	@echo Making all scripts executable
	@find ../scripts -name '*.sh' -type f -exec chmod +x {} +;

format:
	@echo Checking code formatting...
	@find . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\)' \
	-not -path "*gen*" \
	-not -name "*Constants.h" \
	-not -name "*Commands.h" \
	-not -name "*Pins.h" \
	-not -name "*Hardware.h" \
	-not -name "*Version.h" \
	-not -name "*CustomIDs.h" \
	-not -name "*Config.h" \
	-not -name "*MCU.h" \
	-not -name "*Descriptors.*" \
	-exec $(CLANG_FORMAT) -style=file -i {} \;
ifeq ($(CF_FAIL_ON_DIFF), 1)
	git diff -s --exit-code
endif

#debugging
print-%:
	@echo '$*=$($*)'
