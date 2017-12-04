CPP := avr-gcc

CPPFLAGS := \
-funsigned-char \
-funsigned-bitfields \
-fdata-sections \
-ffunction-sections \
-fdata-sections \
-fpack-struct \
-fshort-enums \
-Wall \
-c \
-mrelax \
-std=gnu99 \
-fno-strict-aliasing \
-fno-jump-tables

OPT := -Os
LDFLAGS := -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -Wl,--relax

BUILD_DIR := build/

DEFINES := -DNDEBUG
DEFINES += -DBOARD=BOARD_NONE
DEFINES += -DF_USB=16000000UL
DEFINES += -DARCH=ARCH_AVR8
DEFINES += -DF_CPU=16000000UL
DEFINES += -DBOOT_START_ADDR=0x7000
DEFINES += -DORDERED_EP_CONFIG
DEFINES += -DUSE_STATIC_OPTIONS=0
DEFINES += -DUSB_DEVICE_ONLY
DEFINES += -DNO_SOF_EVENTS
DEFINES += -DUSE_RAM_DESCRIPTORS
DEFINES += -DNO_INTERNAL_SERIAL
DEFINES += -DFIXED_CONTROL_ENDPOINT_SIZE=8
DEFINES += -DDEVICE_STATE_AS_GPIOR
DEFINES += -DFIXED_NUM_CONFIGURATIONS=1
DEFINES += -DNO_DEVICE_REMOTE_WAKEUP
DEFINES += -DNO_DEVICE_SELF_POWER
DEFINES += -DUSB_VENDOR_ID=0x1209
DEFINES += -DUSB_PRODUCT_ID=0x8473

INCLUDE_DIRS := -I"../modules/lufa/"
INCLUDE_DIRS += -I"../modules/"

LUFA_OBJS +=  \
BootloaderHID.o \
Descriptors.o \
../modules/lufa/LUFA/Drivers/USB/Class/Common/HIDParser.o \
../modules/lufa/LUFA/Drivers/USB/Class/Device/HIDClassDevice.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Host_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_R.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_W.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_RW.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Pipe_RW.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.o \
../modules/lufa/LUFA/Drivers/USB/Core/DeviceStandardReq.o \
../modules/lufa/LUFA/Drivers/USB/Core/Events.o \
../modules/lufa/LUFA/Drivers/USB/Core/HostStandardReq.o \
../modules/lufa/LUFA/Drivers/USB/Core/USBTask.o

LUFA_OBJS_WDIR := $(addprefix $(BUILD_DIR),$(LUFA_OBJS:.c=.o))

ifeq ($(MAKECMDGOALS),leonardo)
	TARGET := BootloaderHID_Leonardo
	MCU := atmega32u4
	BOOT_START_ADDR := 0x7000
	DEFINES += -DBOARD_A_LEO
endif

ifeq ($(MAKECMDGOALS),opendeck)
	TARGET := BootloaderHID_OpenDeck
	MCU := atmega32u4
	BOOT_START_ADDR := 0x7000
	DEFINES += -DBOARD_OPEN_DECK
endif

ifeq ($(MAKECMDGOALS),mega)
	TARGET := BootloaderHID_Mega
	MCU := atmega16u2
	BOOT_START_ADDR := 0x3800
	DEFINES += -DBOARD_A_MEGA
endif

DEFINES += -DBOOT_START_ADDR$(BOOT_START_ADDR)

#lufa start
$(LUFA_OBJS_WDIR): $(BUILD_DIR)%.o: %.c
	@mkdir -p $(@D)
	@$(CPP) $(CPPFLAGS) $(DEFINES) $(OPT) $(INCLUDE_DIRS) -mmcu=$(MCU) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -o "$@" "$<" 
	@echo Finished building: $<
#lufa end

# All Target
opendeck leonardo mega: $(TARGET).elf

$(TARGET).elf: $(LUFA_OBJS_WDIR)
	@echo Linking target
	@$(CPP) -o$(TARGET).elf $(LUFA_OBJS_WDIR) -mmcu=$(MCU) $(LDFLAGS) -Wl,--section-start=.text=$(BOOT_START_ADDR)
	@echo Finished building target: $@
	@avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "$(TARGET).elf" "$(TARGET).hex"
	@avr-size "$(TARGET).elf"

# Other Targets
clean:
	rm -rf $(BUILD_DIR)
	rm -rf "$(TARGET).elf" "$(TARGET).a" "$(TARGET).hex"