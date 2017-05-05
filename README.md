# OpenDeck MIDI Platform

OpenDeck is a platform suited both for prototyping and developing custom MIDI controllers. Platform uses class-compliant USB MIDI which makes it compatible with any MIDI software on any OS. Main part of the platform is board on which various components used to build a MIDI controller can be connected. The board supports the following components:

* Buttons
* Encoders
* LEDs (single color or RGB)
* Potentiometers
* FSRs (force-sensitive resistors)

All board configuration is done using OpenDeck SysEx protocol.

## Repository content

* [Firmware](https://github.com/paradajz/OpenDeck/tree/master/src/Firmware)
* [Bootloader](https://github.com/paradajz/OpenDeck/tree/master/src/Bootloader)
* [Schematic (PDF)](https://github.com/paradajz/OpenDeck/blob/master/bin/sch/v1.2.0/OpenDeck-r1.2.0.pdf)
* [BOM](https://github.com/paradajz/OpenDeck/blob/master/bin/sch/v1.2.0/OpenDeck-r1.2.0.csv)
* [Wiki](https://github.com/paradajz/OpenDeck/wiki)

## Credits

Hardware MIDI source code based on [Arduino MIDI library v4.2](https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2) by Francois Best.

USB MIDI and bootloader source code based on [LUFA USB framework](http://www.fourwalledcubicle.com/LUFA.php) by Dean Camera.

Parts of code taken from [Arduino framework](https://github.com/arduino/Arduino).

## Licence

All code is available under GNU GPL v3 licence.