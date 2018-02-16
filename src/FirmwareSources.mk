#determine board variant from target
BOARD_DIR := $(subst fw_,,$(MAKECMDGOALS))

C_SOURCES := 
CPP_SOURCES := 

ifeq ($(BOARD_DIR),pro_micro)
    #pro micro is just a leonardo variant
    BOARD_DIR := leonardo
else ifneq ($(filter fw_16u2 fw_8u2, $(MAKECMDGOALS)), )
    #16u2 and 8u2 are basically same
    BOARD_DIR := xu2
endif

#no lufa for mega or uno
ifeq ($(filter %mega %uno, $(MAKECMDGOALS)), )
    #common for bootloader and firmware
    C_SOURCES := \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_R.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_W.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_RW.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/Template/Template_Pipe_RW.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c \
    modules/lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.c \
    modules/lufa/LUFA/Drivers/USB/Core/DeviceStandardReq.c \
    modules/lufa/LUFA/Drivers/USB/Core/Events.c \
    modules/lufa/LUFA/Drivers/USB/Core/USBTask.c

    #additional sources differ for firmware and bootloader
    ifeq ($(findstring boot,$(MAKECMDGOALS)), boot)
        #bootloader
        C_SOURCES += \
        bootloader/hid/BootloaderHID.c \
        bootloader/hid/Descriptors.c \
        modules/lufa/LUFA/Drivers/USB/Class/Common/HIDParser.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/HIDClassDevice.c
    else
        #firmware
        C_SOURCES += \
        firmware/board/avr/usb/Descriptors.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c
    endif
else
    #no need for lufa
    C_SOURCES :=
endif

ifneq ($(findstring boot,$(MAKECMDGOALS)), boot)
    CPP_SOURCES := \
    firmware/OpenDeck.cpp \
    modules/core/src/HAL/avr/reset/Reset.cpp \
    firmware/database/Database.cpp \
    firmware/database/Layout.cpp \
    modules/dbms/src/DBMS.cpp \
    firmware/interface/analog/Analog.cpp \
    firmware/interface/analog/FSR.cpp \
    firmware/interface/analog/Potentiometer.cpp \
    firmware/interface/cinfo/CInfo.cpp \
    firmware/interface/digital/input/DigitalInput.cpp \
    firmware/interface/digital/input/buttons/Buttons.cpp \
    firmware/interface/digital/input/encoders/Encoders.cpp \
    firmware/interface/digital/output/leds/LEDs.cpp \
    firmware/sysExConf/Layout.cpp \
    firmware/sysExConf/Init.cpp \
    modules/midi/src/MIDI.cpp \
    modules/sysex/src/SysEx.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Analog.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Board.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Buttons.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/DigitalIn.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Encoders.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Init.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/ISR.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/LEDs.cpp \

    #compile display only for mega and teensy at the moment
    ifneq ($(filter fw_mega fw_teensy2pp, $(MAKECMDGOALS)), )
        CPP_SOURCES += \
        firmware/board/avr/display/u8g2_wrapper.cpp \
        firmware/interface/display/UpdateLogic.cpp \
        firmware/interface/display/TextBuild.cpp

        C_SOURCES += $(shell find modules/u8g2/csrc/ -name "*.c")
        #i2c for display communication
        C_SOURCES += modules/i2c/twimaster.c
    endif

    ifeq ($(BOARD_DIR),xu2)
        #xu2 uses different set of sources than other firmwares, overwrite
        CPP_SOURCES := \
        firmware/board/avr/variants/$(BOARD_DIR)/Board.cpp \
        firmware/board/avr/variants/$(BOARD_DIR)/ISR.cpp \
        firmware/board/avr/variants/$(BOARD_DIR)/main.cpp \
        firmware/board/avr/uart/UART_MIDI_1.cpp \
        firmware/board/avr/usb/USB_MIDI.cpp \
        modules/midi/src/MIDI.cpp
    else
        ifneq ($(filter fw_leonardo fw_pro_micro fw_opendeck fw_teensy2pp, $(MAKECMDGOALS)), )
            #these variants all support usb midi and use uart1 for din midi
            CPP_SOURCES += \
            firmware/board/avr/usb/USB_MIDI.cpp \
            firmware/board/avr/uart/UART_MIDI_1.cpp
        else ifneq ($(filter fw_mega fw_uno, $(MAKECMDGOALS)), )
            #no usb midi, uart0 for din midi
            CPP_SOURCES += \
            firmware/board/avr/uart/UART_MIDI_0.cpp
        endif
    endif
else
    #don't use these sources for bootloader
    CPP_SOURCES :=
endif