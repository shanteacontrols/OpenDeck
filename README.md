![Build+Test [Linux]](https://github.com/paradajz/OpenDeck/workflows/Build+Test%20%5BLinux%5D/badge.svg)
![Build+Test [macOS]](https://github.com/paradajz/OpenDeck/workflows/Build+Test%20%5BmacOS%5D/badge.svg)

# OpenDeck MIDI Platform

OpenDeck is a platform suited both for prototyping and developing custom MIDI controllers. Platform uses class-compliant USB MIDI which makes it compatible with any MIDI software on any OS. Main part of the platform is board on which various components used to build a MIDI controller can be connected. OpenDeck supports the following components:

* Buttons
* Encoders
* LEDs (single color or RGB)
* Potentiometers
* FSRs (force-sensitive resistors)
* LCD/OLED displays
* Touchscreen displays (Nextion)

OpenDeck supports sending of both 7-bit and 14-bit Non-Registered Part Numbers (NRPN), latching messages on buttons, various encoding modes for quadrature encoders, LED control using MIDI In, configurable MIDI channels for each component individually, daisy-chaining of boards etc. Full list of supported features can be found [here](https://github.com/paradajz/OpenDeck/wiki/Configurable-features).

All configuration is done using custom SysEx protocol explained in detail on [Wiki page](https://github.com/paradajz/OpenDeck/wiki/SysEx-Configuration) or
using [Web interface](https://paradajz.github.io/OpenDeck).

OpenDeck firmware is compatible with official OpenDeck board, as well as various other boards:

* Arduino Uno
* Arduino Leonardo
* Arduino Pro Micro
* Arduino Mega2560
* Teensy++ 2.0
* STM32F4 Discovery

Support for custom boards can be easily added using [YAML descriptors](https://github.com/paradajz/OpenDeck/wiki/Creating-custom-board-variant).

## Web UI

This repository also contains source code for OpenDeck Web configuration utility written using Angular framework by other developer. Web utility can run in any browser which supports WebMIDI specification (currently Google Chrome and Opera). Utility needs access to MIDI devices so that it can communicate with OpenDeck boards. All communication is done with custom SysEx protocol explained in detail in [Wiki section](https://github.com/paradajz/OpenDeck/wiki/SysEx-Configuration).

Latest version of the UI is always available on [this link](https://paradajz.github.io/OpenDeck). The UI available on this link is hosted on GitHub (via this repository, master branch) and it's compatible only with latest available OpenDeck firmware. If older firmware is used with latest UI, UI tends to crash when it tries to request parameters which aren't available in older firmwares.

Older versions of the UI are also available and can be downloaded from [releases](https://github.com/paradajz/OpenDeck/releases) page.

## Credits

* Hardware MIDI source code based on [Arduino MIDI library v4.2](https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2) by Francois Best.
* USB MIDI source code based on [LUFA USB framework](http://www.fourwalledcubicle.com/LUFA.php) by Dean Camera.

## Licence

All code is available under Apache Licence v2.0.
