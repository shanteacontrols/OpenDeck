#determine board variant from target
board_dir := $(subst fw_,,$(MAKECMDGOALS))

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
modules/sysex/src/SysEx.o \
firmware/board/avr/variants/$(board_dir)/Analog.o \
firmware/board/avr/variants/$(board_dir)/Board.o \
firmware/board/avr/variants/$(board_dir)/Buttons.o \
firmware/board/avr/variants/$(board_dir)/DigitalIn.o \
firmware/board/avr/variants/$(board_dir)/Encoders.o \
firmware/board/avr/variants/$(board_dir)/Init.o \
firmware/board/avr/variants/$(board_dir)/ISR.o \
firmware/board/avr/variants/$(board_dir)/LEDs.o

ifeq ($(MAKECMDGOALS),fw_16u2)
#16u2 uses different set of sources than other firmwares, overwrite
	CPP_OBJECTS := \
firmware/board/avr/variants/$(board_dir)/Board.o \
firmware/board/avr/variants/$(board_dir)/ISR.o \
firmware/board/avr/variants/$(board_dir)/main.o \
firmware/board/avr/uart/UART_MIDI_1.o \
firmware/board/avr/usb/USB_MIDI.o \
modules/midi/src/MIDI.o
endif

ifeq ($(MAKECMDGOALS),fw_leonardo)
	CPP_OBJECTS += \
firmware/board/avr/usb/USB_MIDI.o
endif

ifeq ($(MAKECMDGOALS),fw_opendeck)
	CPP_OBJECTS += \
firmware/board/avr/usb/USB_MIDI.o \
firmware/board/avr/uart/UART_MIDI_1.o
endif

ifeq ($(MAKECMDGOALS),fw_mega)
	CPP_OBJECTS += \
firmware/board/avr/uart/UART_MIDI_0.o
endif