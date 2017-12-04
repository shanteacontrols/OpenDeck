TARGET := ArduinoMIDI_16u2

BUILD_DIR := build/

CPP := avr-gcc
CXX := avr-g++
CXXFLAGS := -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -Wall -c
OPT := -O2
LDFLAGS := -Wl,--section-start=.noinit=0x00800AFD -Wl,--gc-sections -mrelax -Wl,--start-group -Wl,-lm -Wl,--end-group

DEFINES := -DNDEBUG
DEFINES += -DARCH=ARCH_AVR8
DEFINES += -DF_CPU=16000000UL
DEFINES += -DF_USB=16000000UL
DEFINES += -DBOARD=BOARD_NONE
DEFINES += -DEEPROM_SIZE=1024
DEFINES += -DFLASH_SIZE=0x6800
DEFINES += -DMIDI_VENDOR_ID=0x1209
DEFINES += -DMIDI_PRODUCT_ID=0x8472
DEFINES += -DUSE_STATIC_OPTIONS=0
DEFINES += -DUSB_DEVICE_ONLY
DEFINES += -DUSE_FLASH_DESCRIPTORS
DEFINES += -DFIXED_CONTROL_ENDPOINT_SIZE=8
DEFINES += -DFIXED_NUM_CONFIGURATIONS=1
DEFINES += -DINTERRUPT_CONTROL_ENDPOINT
DEFINES += -DMIDI_SYSEX_ARRAY_SIZE=45
DEFINES += -DRING_BUFFER_SIZE=50
DEFINES += -DDBMS_MAX_SECTIONS=7
DEFINES += -DDBMS_MAX_BLOCKS=7
DEFINES += -DSTRING_BUFFER_SIZE=1

MCU := atmega16u2

INCLUDE_FILES := -include "../modules/core/src/Core.h"
INCLUDE_DIRS := -I"../modules/lufa/"
INCLUDE_DIRS += -I"../modules/"

LUFA_OBJS := \
board/usb/Descriptors.o \
../modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.o \
../modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.o \
../modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.o \
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
../modules/lufa/LUFA/Drivers/USB/Core/USBTask.o

LUFA_OBJS_WDIR := $(addprefix $(BUILD_DIR),$(LUFA_OBJS:.c=.o))

CPP_OBJECTS := \
main.o \
board/Board.o \
board/ISR.o \
board/UART_MIDI.o \
board/USB_MIDI.o \
../modules/midi/src/MIDI.o

CPP_OBJECTS_WDIR := $(addprefix $(BUILD_DIR),$(CPP_OBJECTS:.cpp=.o))

#user start
$(CPP_OBJECTS_WDIR): $(BUILD_DIR)%.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(OPT) -mmcu=$(MCU) $(INCLUDE_FILES) $(INCLUDE_DIRS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<
#user end

#lufa start
$(LUFA_OBJS_WDIR): $(BUILD_DIR)%.o: %.c
	@mkdir -p $(@D)
	@$(CPP) $(CXXFLAGS) $(DEFINES) $(OPT) -mmcu=$(MCU) $(INCLUDE_DIRS) -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -o "$@" "$<" 
	@echo Finished building: $<
#lufa end

#targets
all: $(TARGET).elf

$(TARGET).elf: $(LUFA_OBJS_WDIR) $(CPP_OBJECTS_WDIR)
	@$(CXX) -o$(TARGET).elf $(LUFA_OBJS_WDIR) $(CPP_OBJECTS_WDIR) -mmcu=$(MCU)  $(LDFLAGS)
	@echo Finished building target: $@
	@avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "$(TARGET).elf" "../../bin/compiled/$(TARGET).hex"
	@avr-size "$(TARGET).elf"

# Other Targets
clean:
	rm -rf "$(TARGET).elf"
