# OpenDeck Platform

OpenDeck MIDI platform is a combination of microcontroller firmware, PCB board and GUI application.
The platform allows hassle-free building of MIDI controllers and their configuration via MIDI System Exclusive messages,
without any need of reprogramming the chip.

This repository contains MCU code, part list, schematic (circuit and PCBs) and pre-compiled binaries, available for serial
baud rates: 38400, for use with virtual MIDI cable software and serial-to-midi converter, and 31250 for use with native
MIDI.

Code base is built upon OpenDeck library, Ownduino library (minimal fork of Arduino core library)
and modified Arduino MIDI library 3.2. All code is available under GNU GPL v3 licence.


# Configurable features

## Hardware parameters

* Board type (OpenDeck reference board, Tannin board, or user-defineable board in `HardwareControl.cpp`)
* Enable/disable buttons
* Enable/disable LEDs
* Enable/disable potentiometers

## Features

### MIDI features

* Enable/disable running status
* Enable/disable standard note off

### Button features

* Enable/disable long-press mode

### LED features

* Enable/disable start-up routine
* Enable/disable blinking

### Potentiometer features

* Enable/disable potentiometer notes

### MIDI channels

* Button note channel
* Long-press button note channel
* Program change button channel
* Potentiometer CC channel
* Potentiometer program change channel
* Potentiometer note channel
* Input MIDI channel

## Button configuration

* Hardware parameters: long-press time
* Button type (momentary/latching)
* Enable/disable program change send instead of notes
* Button MIDI note

## Potentiometer configuration

* Hardware parameters: (reserved/no parameters yet)
* Enable/disable potentiometer
* Enable/disable program change send instead of CC
* Invert potentiometer
* CC/Program change number
* Lower CC/PP limit
* Upper CC/PP limit

## LED configuration

* Hardware parameters: total LED number, blink time, start-up switch time, start-up routine
* LED activation note
* LED start-up number
* Test LED state (constant on/off, blink on/off)


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

After that, board type must be specified. When using OpenDeck reference board, set it using the following message:

F0 00 53 43 01 00 00 00 00 01 F7


Each SysEx message (except for hello world) uses the same format:

F0 ID ID ID WISH AMOUNT MESSAGE_TYPE MESSAGE_SUBTYPE PARAMETER_ID NEW_PARAMETER_ID F7

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

* Hardware configuration (0x00)
* Features (0x01)
* MIDI channel (0x02)
* Button (0x03)
* Potentiometer (0x04)
* LED (0x05)
* All (0x07)

### MESSAGE_SUBTYPE

Features, buttons, potentiometers, LEDs are the only types of message which have a sub-type. When dealing
with other message types, code 0x00 must be specified as sub-type.

Buttons:
* Hardware parameter (0x00)
* Type (0x01)
* Program change enabled (0x02)
* Button note (0x03)

Potentiometers:
* Hardware parameter (0x00)
* Pot enabled (0x01)
* Program change enabled (0x02)
* Pot inverted (0x03)
* CC/PP number (0x04)
* Lower CC/PP limit (0x05)
* Upper CC/PP limit (0x06)

LEDs:
* Hardware parameter (0x00)
* Activation note (0x01)
* Start-up number (0x02)
* State (0x03)


For more information, see examples in /examples folder.