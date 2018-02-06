#determine board variant from target
BOARD_DIR := $(subst fw_,,$(MAKECMDGOALS))

ifeq ($(BOARD_DIR),pro_micro)
    #pro micro is just a leonardo variant
    BOARD_DIR := leonardo
else ifneq ($(filter fw_16u2 fw_8u2, $(MAKECMDGOALS)), )
    #16u2 and 8u2 are basically same
    BOARD_DIR := xu2
endif

ifneq ($(findstring boot,$(MAKECMDGOALS)), boot)
    CPP_OBJECTS := \
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
    modules/midi/src/MIDI.cpp \
    modules/sysex/src/SysEx.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Analog.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Board.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Buttons.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/DigitalIn.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Encoders.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/Init.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/ISR.cpp \
    firmware/board/avr/variants/$(BOARD_DIR)/LEDs.cpp

    ifeq ($(BOARD_DIR),xu2)
        #xu2 uses different set of sources than other firmwares, overwrite
        CPP_OBJECTS := \
        firmware/board/avr/variants/$(BOARD_DIR)/Board.cpp \
        firmware/board/avr/variants/$(BOARD_DIR)/ISR.cpp \
        firmware/board/avr/variants/$(BOARD_DIR)/main.cpp \
        firmware/board/avr/uart/UART_MIDI_1.cpp \
        firmware/board/avr/usb/USB_MIDI.cpp \
        modules/midi/src/MIDI.cpp
    else
        ifneq ($(filter fw_leonardo fw_pro_micro fw_opendeck fw_teensy2pp, $(MAKECMDGOALS)), )
            #these variants all support usb midi and use uart1 for din midi
            CPP_OBJECTS += \
            firmware/board/avr/usb/USB_MIDI.cpp \
            firmware/board/avr/uart/UART_MIDI_1.cpp
        else ifneq ($(filter fw_mega fw_uno, $(MAKECMDGOALS)), )
            #no usb midi, uart0 for din midi
            CPP_OBJECTS += \
            firmware/board/avr/uart/UART_MIDI_0.cpp
        endif
    endif
else
    #don't use these sources for bootloader
    CPP_OBJECTS :=
endif