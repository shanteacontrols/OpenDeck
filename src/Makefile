#includes
include Defines.mk
include Sources.mk
include Upload.mk

#target location
TARGET := build/$(MAKECMDGOALS)

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
OPT := -Os

ifneq ($(filter boot_%, $(MAKECMDGOALS)), )
    #make sure to link .text at correct address in bootloader
    LDFLAGS += -Wl,--section-start=.text=$(BOOT_START_ADDR)
endif

LEN_APPEND := 0

#custom linker script
LDFLAGS += -T application/board/avr/linker/$(MCU).x

#append length only in firmware
ifeq ($(findstring fw,$(MAKECMDGOALS)), fw)
    LEN_APPEND := 1
endif

#make sure all objects are located in build directory
OBJECTS := $(addprefix build/,$(SOURCES))
#also make sure objects have .o extension
OBJECTS := $(OBJECTS:.c=.o)
OBJECTS := $(OBJECTS:.cpp=.o)

#include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)

#targets
fw_opendeck \
fw_leonardo \
fw_mega \
fw_pro_micro \
fw_uno \
fw_teensy2pp \
fw_kodama \
fw_tannin \
fw_bergamot \
fw_16u2 \
fw_8u2 \
boot_opendeck \
boot_leonardo \
boot_mega \
boot_teensy2pp \
boot_kodama \
boot_tannin \
boot_16u2: $(TARGET).elf

#use windows binary on wsl since HID access isn't possible in wsl
ifeq ($(findstring Microsoft,$(shell uname -r)), Microsoft)
    UPLOAD_BIN := @cmd.exe /c "..\bin\dfu\loader\hid_bootloader_loader_win.exe"
endif

build/%.o: %.c
	@mkdir -p $(@D)
	@$(CPP) $(CXXFLAGS) $(addprefix -D,$(DEFINES)) $(OPT) -mmcu=$(MCU) $(INCLUDE_DIRS) -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<

build/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -std=c++11 $(addprefix -D,$(DEFINES)) $(OPT) -mmcu=$(MCU) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<

$(TARGET).elf: target_print $(OBJECTS)
	@#link
	@$(CXX) -o$(TARGET).elf $(OBJECTS) -mmcu=$(MCU) $(LDFLAGS) -Wl,-Map,$(TARGET).map
	@echo Finished building target: $@
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

#firmware upload with avrdude
upload:
	@$(AVR_DUDE_BIN_PATH) -p $(shell cat build/MCU) -P /dev/$(PORT) -b 19200 -c $(AVRDUDE_PROGRAMMER) -e -U lock:w:$(FUSE_UNLOCK):m -U efuse:w:$(FUSE_EXT):m -U hfuse:w:$(FUSE_HIGH):m -U lfuse:w:$(FUSE_LOW):m
	@$(AVR_DUDE_BIN_PATH) -p $(shell cat build/MCU) -P /dev/$(PORT) -b 19200 -c $(AVRDUDE_PROGRAMMER) -U flash:w:build/$(shell cat build/TARGET).hex -U lock:w:$(FUSE_LOCK):m

#firmware update with bootloader
uploadboot:
	$(UPLOAD_BIN) $(shell cat build/MCU) build/$(shell cat build/TARGET).hex

#lists all serial ports - used to find out where the programmer is connected
ser_list:
	dmesg | grep tty

#other targets
clean:
	@echo Cleaning up.
	@rm -rf build/ ../Documentation/

target_print:
	@echo Building target $(MAKECMDGOALS)
	@mkdir -p build
	@#create a file with contents being mcu name (for upload process)
	@echo $(MCU) > build/MCU
	@#create a file with contents being target name (for upload process)
	@echo $(MAKECMDGOALS) > build/TARGET

#debugging
print-%:
	@echo '$*=$($*)'
