[![Build Status](https://travis-ci.org/paradajz/OpenDeck.svg?branch=master)](https://travis-ci.org/paradajz/OpenDeck)

# OpenDeck MIDI Platform

OpenDeck is a platform suited both for prototyping and developing custom MIDI controllers. Platform uses class-compliant USB MIDI which makes it compatible with any MIDI software on any OS. Main part of the platform is board on which various components used to build a MIDI controller can be connected. OpenDeck supports the following components:

* Buttons
* Encoders
* LEDs (single color or RGB)
* Potentiometers
* FSRs (force-sensitive resistors)
* LCD/OLED displays (Arduino Mega and Teensy++ 2.0 only)

OpenDeck supports sending of both 7-bit and 14-bit Non-Registered Part Numbers (NRPN), latching messages on buttons, various encoding modes for quadrature encoders, LED control using MIDI In, configurable MIDI channels etc. Full list of supported features can be found [here](https://github.com/paradajz/OpenDeck/wiki/Configurable-features).

All configuration is done using custom SysEx protocol explained in detail on [Wiki page](https://github.com/paradajz/OpenDeck/wiki/SysEx-Configuration) or
using [Web interface](https://config.shanteacontrols.com/).

OpenDeck firmware is compatible with official OpenDeck board, as well as various other boards:

* Arduino Uno
* Arduino Leonardo
* Arduino Pro Micro
* Arduino Mega2560
* Teensy++ 2.0

## Coming soon

* Support for advanced I2C sensors

## Credits

* Hardware MIDI source code based on [Arduino MIDI library v4.2](https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2) by Francois Best.
* USB MIDI and bootloader source code based on [LUFA USB framework](http://www.fourwalledcubicle.com/LUFA.php) by Dean Camera.

## Licence

All code is available under GNU GPL v3 licence.
