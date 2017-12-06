CPP_OBJECTS := \
firmware/OpenDeck.o \
modules/core/src/HAL/avr/reset/Reset.o \
firmware/database/Database.o \
firmware/database/Layout.o \
modules/dbms/src/DBMS.o \
firmware/interface/analog/Analog.o \
firmware/interface/analog/FSR.o \
firmware/interface/analog/Potentiometer.o \
firmware/interface/cinfo/CInfo.o \
firmware/interface/digital/input/DigitalInput.o \
firmware/interface/digital/input/buttons/Buttons.o \
firmware/interface/digital/input/encoders/Encoders.o \
firmware/interface/digital/output/leds/LEDs.o \
modules/midi/src/MIDI.o \
modules/sysex/src/SysEx.o

ifeq ($(MAKECMDGOALS),fw_16u2)
#16u2 uses different set of sources than other firmwares, overwrite
	CPP_OBJECTS := \
firmware/board/avr/variants/arduino16u2/Board.o \
firmware/board/avr/variants/arduino16u2/ISR.o \
firmware/board/avr/variants/arduino16u2/main.o \
modules/midi/src/MIDI.o \
firmware/board/avr/uart/UART_MIDI_1.o \
firmware/board/avr/usb/USB_MIDI.o
endif

ifeq ($(MAKECMDGOALS),fw_leonardo)
	CPP_OBJECTS += \
firmware/board/avr/variants/leonardo/Analog.o \
firmware/board/avr/variants/leonardo/Board.o \
firmware/board/avr/variants/leonardo/Buttons.o \
firmware/board/avr/variants/leonardo/DigitalIn.o \
firmware/board/avr/variants/leonardo/Encoders.o \
firmware/board/avr/variants/leonardo/Init.o \
firmware/board/avr/variants/leonardo/ISR.o \
firmware/board/avr/variants/leonardo/LEDs.o \
firmware/board/avr/usb/USB_MIDI.o
endif

ifeq ($(MAKECMDGOALS),fw_opendeck)
	CPP_OBJECTS += \
firmware/board/avr/variants/opendeck/Analog.o \
firmware/board/avr/variants/opendeck/Board.o \
firmware/board/avr/variants/opendeck/Buttons.o \
firmware/board/avr/variants/opendeck/DigitalIn.o \
firmware/board/avr/variants/opendeck/Encoders.o \
firmware/board/avr/variants/opendeck/Init.o \
firmware/board/avr/variants/opendeck/ISR.o \
firmware/board/avr/variants/opendeck/LEDs.o \
firmware/board/avr/usb/USB_MIDI.o \
firmware/board/avr/uart/UART_MIDI_1.o
endif

ifeq ($(MAKECMDGOALS),fw_mega)
#no usb support for mega - it uses 16u2 as translator
	CPP_OBJECTS += \
firmware/board/avr/variants/mega/Analog.o \
firmware/board/avr/variants/mega/Board.o \
firmware/board/avr/variants/mega/Buttons.o \
firmware/board/avr/variants/mega/DigitalIn.o \
firmware/board/avr/variants/mega/Encoders.o \
firmware/board/avr/variants/mega/Init.o \
firmware/board/avr/variants/mega/ISR.o \
firmware/board/avr/variants/mega/LEDs.o \
firmware/board/avr/uart/UART_MIDI_0.o
endif