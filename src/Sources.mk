#determine board variant from target
BOARD_DIR := $(subst fw_,,$(MAKECMDGOALS))

ifeq ($(BOARD_DIR),pro_micro)
    #pro micro is just a leonardo variant
    BOARD_DIR := leonardo
else ifneq ($(filter fw_16u2 fw_8u2, $(MAKECMDGOALS)), )
    #16u2 and 8u2 are basically same
    BOARD_DIR := xu2
endif

SOURCES :=

#lufa sources
#no lufa for mega or uno
ifeq ($(filter %mega %uno, $(MAKECMDGOALS)), )
    #common for bootloader and firmware
    SOURCES += \
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
        SOURCES += \
        bootloader/hid/BootloaderHID.c \
        bootloader/hid/Descriptors.c \
        modules/lufa/LUFA/Drivers/USB/Class/Common/HIDParser.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/HIDClassDevice.c
    else
        #firmware
        SOURCES += \
        application/board/avr/usb/Descriptors.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/AudioClassDevice.c \
        modules/lufa/LUFA/Drivers/USB/Class/Device/MIDIClassDevice.c
    endif
endif

#only for firmware
ifneq ($(findstring boot,$(MAKECMDGOALS)), boot)
    ifeq ($(BOARD_DIR),xu2)
        #xu2 uses different set of sources than other firmwares
        SOURCES += \
        application/board/avr/variants/$(BOARD_DIR)/xu2.cpp \
        application/board/avr/variants/$(BOARD_DIR)/ISR.cpp \
        application/board/avr/uart/UART_MIDI_1.cpp \
        application/board/avr/usb/USB_MIDI.cpp \
        modules/midi/src/MIDI.cpp
    else
        SOURCES += \
        application/OpenDeck.cpp \
        modules/core/src/HAL/avr/reset/Reset.cpp \
        application/database/Database.cpp \
        modules/dbms/src/DBMS.cpp \
        application/interface/analog/Analog.cpp \
        application/interface/analog/FSR.cpp \
        application/interface/analog/Potentiometer.cpp \
        application/interface/cinfo/CInfo.cpp \
        application/interface/digital/input/DigitalInput.cpp \
        application/interface/digital/input/buttons/Buttons.cpp \
        application/interface/digital/input/encoders/Encoders.cpp \
        application/interface/digital/output/leds/LEDs.cpp \
        application/sysExConf/Handling.cpp \
        modules/midi/src/MIDI.cpp \
        modules/sysex/src/SysEx.cpp \
        application/board/avr/variants/$(BOARD_DIR)/Init.cpp \
        application/board/avr/variants/$(BOARD_DIR)/ISR.cpp \
        application/board/avr/variants/Common.cpp \
        application/board/common/analog/input/Common.cpp \
        application/board/common/digital/input/encoders/Common.cpp \

        #board specific
        ifneq ($(filter USE_MUX, $(DEFINES)), )
        SOURCES += \
        application/board/common/analog/input/Mux.cpp
        endif

        ifneq ($(filter IN_MATRIX, $(DEFINES)), )
        SOURCES += \
        application/board/common/digital/input/matrix/InMatrix.cpp \
        application/board/common/digital/input/buttons/Matrix.cpp \
        application/board/common/digital/input/encoders/Matrix.cpp
        else
        SOURCES += \
        application/board/common/digital/input/direct/Direct.cpp \
        application/board/common/digital/input/buttons/DirectIn.cpp \
        application/board/common/digital/input/encoders/DirectIn.cpp
        endif

        ifneq ($(filter OUT_MATRIX, $(DEFINES)), )
        SOURCES += \
        application/board/common/digital/output/matrix/OutMatrix.cpp \
        application/board/common/digital/output/leds/Matrix.cpp
        else
        SOURCES += \
        application/board/common/digital/output/leds/DirectOut.cpp
        endif

        ifneq ($(filter fw_leonardo fw_pro_micro fw_opendeck fw_teensy2pp fw_kodama, $(MAKECMDGOALS)), )
            #these variants all support usb midi and use uart1 for din midi
            SOURCES += \
            application/board/avr/usb/USB_MIDI.cpp \
            application/board/avr/uart/UART_MIDI_1.cpp
        else ifneq ($(filter fw_mega fw_uno, $(MAKECMDGOALS)), )
            #no usb midi, uart0 for din midi
            SOURCES += \
            application/board/avr/uart/UART_MIDI_0.cpp
        endif

        #compile display only for mega and teensy at the moment
        ifneq ($(filter fw_mega fw_teensy2pp, $(MAKECMDGOALS)), )
            SOURCES += \
            application/board/avr/display/u8g2_wrapper.cpp \
            application/interface/display/UpdateLogic.cpp \
            application/interface/display/TextBuild.cpp

            #compile all sources in u8g2 dir for now
            SOURCES += $(shell find modules/u8g2/csrc/ -name "*.c")
            #i2c for display communication
            SOURCES += modules/i2c/twimaster.c
        endif
    endif
endif