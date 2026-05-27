# OpenDeck MIDI Platform

OpenDeck is a platform for building MIDI and OSC controllers designed to solve two problems when building a controller:

* Coding - there is no need to code anything in order to make changes to the controller or to make it work
* Making the controller behave the way you want it to - the firmware supports a huge number of configurable parameters, all configurable in a Web browser

Solving these two goals means you can forget about implementation details and focus on having your custom controller built as fast as possible.

*Click the image below for a demo video of the [OpenDeck configurator](https://config.shanteacontrols.com)*

[![Watch the video](https://img.youtube.com/vi/7X2LC0JMfAU/maxresdefault.jpg)](https://youtu.be/7X2LC0JMfAU)

Configurator is available either [online](https://config.shanteacontrols.com) or [offline](https://github.com/shanteacontrols/OpenDeckUI/releases).

*Some of the controllers built with OpenDeck*

![](bin/img/readme/gallery.png)

## Features

The following components are supported:

* Buttons
* Encoders
* Single-color LEDs and other types of outputs such as relays or transistors
* Potentiometers
* FSRs (force-sensitive resistors)
* LCD/OLED displays
* Touchscreen displays (Nextion)

OpenDeck supports sending of both 7-bit and 14-bit Non-Registered Parameter Numbers (NRPN), latching messages on buttons, various encoding modes for quadrature encoders, output control using MIDI In, configurable MIDI channels for each component individually, Bluetooth MIDI, OSC via Ethernet etc. Full list of supported features can be found [here](https://github.com/shanteacontrols/OpenDeck/wiki/Configurable-features).

If the feature you want isn't supported yet, don't feel discouraged. Open up a [discussion thread](https://github.com/shanteacontrols/OpenDeck/discussions) and let's talk about it.

## Official board

Currently, there is only one official board available to buy: the L variant, where "L" stands for "large". The board is available on [Lectronz store](https://lectronz.com/products/opendeck-l/).

![](https://shanteacontrols.com/images/landing/header/od3_1.webp)

This board is based on Raspberry Pi RP2040 microcontroller and features the following:

* 128 digital inputs
* 64 digital outputs
* 64 analog inputs
* DIN MIDI
* USB MIDI (USB C)
* Connector for touchscreen
* Connector for I2C OLED display
* Additional connectors with 3V+GND and 5V+GND connections

## Supported boards

The firmware supports various other boards, both small and large:

| Board | MIDI | OSC |
| --- | --- | --- |
| [Adafruit Grand Central M4 Express](https://www.adafruit.com/product/4064) | USB, DIN | No |
| [Arduino Nano 33 BLE](https://store.arduino.cc/products/arduino-nano-33-ble) | USB, DIN, BLE | No |
| [nRF52840 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) | USB, DIN, BLE | No |
| [nRF5340 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf5340-dk) | USB, DIN, BLE | No |
| [Olimex ESP32-POE](https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/open-source-hardware) | No | Ethernet |
| [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) | USB, DIN | No |
| [Raspberry Pi Pico 2](https://www.raspberrypi.com/products/raspberry-pi-pico-2/) | USB, DIN | No |
| [ST Discovery F407VG](https://www.st.com/en/evaluation-tools/stm32f4discovery.html) | USB, DIN | No |
| [ST Nucleo F767ZI](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html) | DIN | Ethernet |
| [ST Nucleo H753ZI](https://www.st.com/en/evaluation-tools/nucleo-h753zi.html) | DIN | Ethernet |
| [STM32F411 Black Pill V3+](https://www.adafruit.com/product/4877) with populated external flash | USB, DIN | No |
| [Teensy 4.0](https://www.pjrc.com/store/teensy40.html) | USB, DIN | No |
| [Teensy 4.1](https://www.pjrc.com/store/teensy41.html) | USB, DIN | No |
| [Waveshare Core405R](https://www.waveshare.com/core405r.htm) | USB, DIN | No |
| [wESP32](https://wesp32.com/) | No | Ethernet |

For more details on supported boards, check the [wiki page](https://github.com/shanteacontrols/OpenDeck/wiki). A page with [instructions on how to flash OpenDeck firmware](https://github.com/shanteacontrols/OpenDeck/wiki/Flashing-the-OpenDeck-firmware) to supported boards is also available. Support for custom boards [can also be added](https://github.com/shanteacontrols/OpenDeck/wiki/Creating-custom-board-variant).

## Documentation

Available on dedicated [Wiki section](https://github.com/shanteacontrols/OpenDeck/wiki/).

## Discussion

Need help? Want to show off your OpenDeck-based builds? Head over to [Discussions](https://github.com/shanteacontrols/OpenDeck/discussions)!

## License

OpenDeck source code is available under Apache License v2.0. Zephyr RTOS, which OpenDeck is built on top of, is also licensed under Apache License v2.0.

Third-party components with different licenses:

* `zlibs`: MIT
* `u8g2`: Two-clause BSD
