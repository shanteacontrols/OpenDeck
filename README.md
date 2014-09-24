# OpenDeck Platform

OpenDeck MIDI platform is a combination of microcontroller firmware, PCB board and GUI application.
The platform allows hassle-free building of MIDI controllers and their configuration via MIDI System Exclusive messages,
without any need of reprogramming the chip.

This repository contains MCU code, part list, schematic (circuit and PCBs) and pre-compiled binaries, available for serial
baud rates: 38400, for use with virtual MIDI cable software and serial-to-midi converter, and 31250 for use with native
MIDI chip (USB MIDI PCB under schematics/AU123).

Code base is built upon OpenDeck library, Ownduino library (minimal fork of Arduino core library)
and modified Arduino MIDI library 3.2. All code is available under GNU GPL v3 licence.


## Configurable features

### MIDI channels:
* Button note channel
* Long-press note channel
* Potentiometer CC channel
* Potentiometer note channel
* Input MIDI channel

### Hardware parameters
* Board type (OpenDeck reference board, Tannin board, or user-defineable board in `HardwareControl.cpp`)
* Long-press time for buttons
* LED blink time
* Total number of LEDs
* Start-up routine LED switch time
* Start-up routine pattern

### Free pins
* Reference board contains 4 unused pins, A, B, C and D. Each pin can be configured as input or output,
resulting in extra button or LED row, except for pin C, which can only be configured as output.
Configuring free pins can lead to maximum of 56 buttons or 64 LEDs, depending on configuration.

### Software features (enable/disable)
* MIDI running status
* Standard MIDI note off
* Encoder notes
* Potentiometer notes (6 notes from each pot, depending on its position)
* Long-press
* LED blinking
* Start-up routine

### Hardware features (enable/disable)
* Buttons
* Pots
* Encoders
* LEDs

### Button configuration
* Button type (momentary/latching)
* Button MIDI note

### Potentiometer configuration
* Enable/disable potentiometer
* Invert potentiometer
* MIDI CC number
* Lower CC limit
* Upper CC limit

### LED configuration
* MIDI activation note for each LED
* LED start number for start-up routine
* LED testing (turning LED on/off and blink test)

### Data restoration
* Restoration of single parameter back to default (ie. restore potentiometer 6 CC number to default)
* Restoration of all parameters within message type (ie. restore all button notes to default)
* Complete factory reset of all values

## Configuration

All configuration is done using MIDI System Exclusive messages.
Before attempting to change parameters of OpenDeck controller, a "Hello World" message must be sent to controller:

F0 00 53 43 F7

F0 is a SysEx start, 00, 50 and 43 are ID bytes, and F7 is SysEx end. After the controller receives the message, it
enables full manipulation using SysEx protocol, and responds with the same message, with an extra ACK byte at the end, 0x41:

F0 00 53 43 41 F7

After that, board type must be specified. When using OpenDeck reference board, set it using the following message:

F0 00 53 43 01 00 54 00 00 01 F7


Each SysEx message (except for hello world) uses the same format:

F0 ID ID ID WISH AMOUNT MESSAGE_TYPE MESSAGE_SUBTYPE PARAMETER_ID NEW_PARAMETER_ID F7

### ID
* Byte 1: 0x00
* Byte 2: 0x53
* Byte 3: 0x43

### WISH
* Get (0x00)
Used to request data from the controller
* Set (0x01)
Changing parameters
* Restore (0x02)
Data restoration to default state

### AMOUNT
* Single (0x00)
Only one parameter
* All (0x01)
All parameters

### MESSAGE_TYPE
There's a total of 10 message types available.

* MIDI channel (0x00)
* Hardware parameter (0x01)
* Free pin (0x02)
* Software feature (0x03)
* Hardware feature (0x04)
* Button (0x05)
* Potentiometer (0x06)
* Encoder (0x07)
* LED (0x08)
* All (0x09)

### MESSAGE_SUBTYPE

Buttons, potentiometers and LEDs are the only types of message which have a sub-type. When dealing
with other message types, code 0x00 must be specified as sub-type.

Buttons:
* Button type (0x00)
* Button note (0x01)

Potentiometers:
* Pot enabled (0x00)
* Pot inverted (0x01)
* CC number (0x02)
* Lower CC limit (0x03)
* Upper CC limit (0x04)

LEDs:
* Activation note (0x00)
* Start-up number (0x01)
* State (0x02)


For parameters and more info, `see SysEx.h`