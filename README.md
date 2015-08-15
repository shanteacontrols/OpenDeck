# OpenDeck Platform

OpenDeck MIDI platform is a combination of microcontroller firmware, PCB board and GUI application.
The platform allows hassle-free building of MIDI controllers and their configuration via MIDI System Exclusive messages,
without any need of reprogramming the chip.

This repository contains MCU code, part list and schematics (circuit and PCB).

Code base is built upon OpenDeck library, modified Arduino MIDI library 3.2 and Teensyduino. All code is available under GNU GPL v3 licence.


# Configurable features

## MIDI channels

* Note
* Program change
* Control change
* Pitch bend
* MIDI input

## Features

### MIDI features

* Enable/disable standard note off


## Button configuration

* Button type (momentary/latching)
* Enable/disable program change send instead of notes
* Button MIDI note

## Analog configuration

* Hardware parameters: (reserved/no parameters yet)
* Enable/disable analog input
* Analog input type (only potentiometer at the moment)
* Invert analog value
* CC number
* Lower CC limit
* Upper CC limit

## LED configuration

* Hardware parameters: total LED number, blink time, start-up switch time, start-up routine, fade speed
* LED activation note
* LED start-up number
* Test LED state (constant on/off, blink on/off)

## Encoder configuration

* Hardware parameters (reserved/no parameters yet)
* Enable/disable encoder
* Invert encoder value
* Change encoder CC number

## Data restoration

* Restoration of single parameter back to default (ie. restore potentiometer 6 CC number to default)
* Restoration of all parameters within message type (ie. restore all button notes to default)
* Complete factory reset of all values

# SysEx configuration

All configuration is done using MIDI System Exclusive messages.
Before attempting to change parameters of OpenDeck controller, a "Hello World" message must be sent to controller:

F0 00 53 43 F7

F0 is a SysEx start, 00, 50 and 43 are ID bytes, and F7 is SysEx end. After the controller receives the message, it
enables full manipulation using SysEx protocol, and responds with the same message, with an extra ACK byte at the end, 0x41:

F0 00 53 43 41 F7

Each SysEx message (except for hello world) uses the same format:

F0 ID ID ID WISH AMOUNT MESSAGE_TYPE MESSAGE_SUBTYPE PARAMETER_ID* NEW_PARAMETER_ID* F7

Depending on wish and amount, PARAMETER_ID and NEW_PARAMETER_ID might not be neccessary.

## ID

* Byte 1: 0x00
* Byte 2: 0x53
* Byte 3: 0x43

## WISH

* Get (0x00)
Used to request data from the controller
* Set (0x01)
Changing parameters
* Restore (0x02)
Data restoration to default state

## AMOUNT
* Single (0x00)
Only one parameter
* All (0x01)
All parameters

### MESSAGE_TYPE
There's a total of 7 message types available.

* MIDI channel (0x00)
* Features (0x01)
* Buttons (0x02)
* Analog (0x03)
* LEDs (0x04)
* Encoders (0x05)
* All (0x06)

### MESSAGE_SUBTYPE

Features, buttons, analog, LEDs and encoders have sub-type. When dealing
with other message types, code 0x00 must be specified as sub-type.

Features:

* MIDI features (0x00)
* Button features (0x01)
* LED features (0x02)
* Potentiometer features (0x03)

Buttons:
* Hardware parameter (0x00)
* Type (0x01)
* Program change enabled (0x02)
* Button note (0x03)

Analog:
* Hardware parameter (0x00)
* Analog enabled (0x01)
* Analog type (0x02)
* Analog inverted (0x03)
* CC number (0x04)
* Lower CC limit (0x05)
* Upper CC limit (0x06)

LEDs:
* Hardware parameter (0x00)
* Activation note (0x01)
* Start-up number (0x02)
* State (0x03)

Encoders:
* Hardware parameter (0x00)
* Encoder enabled (0x01)
* Encoder inverted (0x02)
* Encoder CC number (0x05)


For more information, see examples in /examples folder.