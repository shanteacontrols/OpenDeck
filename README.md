[![project chat](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://shanteacontrols.zulipchat.com)

![Firmware build [Linux]](https://github.com/paradajz/OpenDeck/workflows/Firmware%20build%20%5BLinux%5D/badge.svg) ![Test [Linux]](https://github.com/paradajz/OpenDeck/workflows/Test%20%5BLinux%5D/badge.svg) ![Code formatting](https://github.com/paradajz/OpenDeck/workflows/Code%20formatting/badge.svg)

# OpenDeck MIDI Platform

OpenDeck is a platform suited both for prototyping and developing custom MIDI controllers with easy to use web configurator.

![](https://github.com/paradajz/OpenDeck/blob/master/bin/img/webui.png?raw=true)

Platform uses class-compliant USB MIDI which makes it compatible with any MIDI software on any OS. Main part of the platform is board on which various components used to build a MIDI controller can be connected. OpenDeck supports the following components:

* Buttons
* Encoders
* LEDs (single color or RGB)
* Potentiometers
* FSRs (force-sensitive resistors)
* LCD/OLED displays
* Touchscreen displays (Nextion and Viewtech/Stone)

OpenDeck supports sending of both 7-bit and 14-bit Non-Registered Part Numbers (NRPN), latching messages on buttons, various encoding modes for quadrature encoders, LED control using MIDI In, configurable MIDI channels for each component individually etc. Full list of supported features can be found [here](https://github.com/paradajz/OpenDeck/wiki/Configurable-features).

OpenDeck firmware is compatible with [official OpenDeck board](https://www.tindie.com/products/paradajz/opendeck-diy-midi-platform/), as well as various other boards:

* Arduino Mega2560
* Teensy++ 2.0
* STM32F4 Discovery
* STM32F401 Black Pill
* STM32F411 Black Pill

Support for custom boards can be easily added using [YAML descriptors](https://github.com/paradajz/OpenDeck/wiki/Creating-custom-board-variant).

## Web configurator

Source code for OpenDeck web configurator is located in [OpenDeckUI repository](https://github.com/paradajz/OpenDeckUI). It's written by [wyrd-code](https://github.com/wyrd-code/). Web configurator can run in any browser which supports WebMIDI specification (any browser based on Chromium). Utility needs access to MIDI devices so that it can communicate with OpenDeck boards. All communication is done with custom SysEx protocol explained in detail in [Wiki section](https://github.com/paradajz/OpenDeck/wiki/SysEx-Configuration).

Latest stable version of the UI is always available on [this link](https://paradajz.github.io/OpenDeck).

## Credits

* Hardware MIDI source code based on [Arduino MIDI library v4.2](https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2) by Francois Best.
* USB MIDI source code based on [LUFA USB framework](http://www.fourwalledcubicle.com/LUFA.php) by Dean Camera.

## Licence

All code is available under Apache Licence v2.0.
