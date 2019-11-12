/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

///
/// \brief Holds current version of hardware.
/// Can be overriden during build process to compile
/// the firmware for different hardware revision of the board.
/// @{

#ifndef HARDWARE_VERSION_MAJOR
#define HARDWARE_VERSION_MAJOR  1
#endif

#ifndef HARDWARE_VERSION_MINOR
#define HARDWARE_VERSION_MINOR  0
#endif

/// @}

///
/// \brief Specifies which MCU this board variant uses.
///
#define MCU                             "at90usb1286"

///
/// \brief Indicates that the board supports USB MIDI.
///
#define USB_MIDI_SUPPORTED

///
/// \brief Defines total number of available UART interfaces on board.
///
#define UART_INTERFACES                 1

///
/// \brief Indicates that the board supports DIN MIDI.
///
#define DIN_MIDI_SUPPORTED

///
/// \brief Defines UART channel used for DIN MIDI.
///
#define UART_MIDI_CHANNEL               0

///
/// \brief Constant used to debounce button readings.
///
#define BUTTON_DEBOUNCE_COMPARE         0b11110000

///
/// brief Total number of analog components.
///
#define MAX_NUMBER_OF_ANALOG            8

///
/// \brief Maximum number of buttons.
///
#define MAX_NUMBER_OF_BUTTONS           16

///
/// \brief Indicates that the board supports LEDs.
///
#define LEDS_SUPPORTED

///
/// \brief Maximum number of LEDs.
///
#define MAX_NUMBER_OF_LEDS              16

///
/// \brief Maximum number of RGB LEDs.
/// One RGB LED requires three standard LED connections.
///
#define MAX_NUMBER_OF_RGB_LEDS          (MAX_NUMBER_OF_LEDS/3)

///
/// \brief Maximum number of encoders.
/// Total number of encoders is total number of buttons divided by two.
///
#define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)

///
/// \brief Indicates that the board supports display interface.
///
#define DISPLAY_SUPPORTED

///
/// \brief If touchscreen isn't supported, set MAX_TOUCHSCREEN_BUTTONS to zero.
///
#define MAX_TOUCHSCREEN_BUTTONS         0

///
/// \brief Total available bytes for data in EEPROM.
///
#define EEPROM_SIZE                     4096