/*

Copyright 2015-2020 Igor Petrovic

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
/// \brief Defines total number of available UART interfaces on board.
///
#define UART_INTERFACES                 2

///
/// \brief Indicates that the board supports DIN MIDI.
///
#define DIN_MIDI_SUPPORTED

///
/// \brief Defines UART channel used for DIN MIDI.
///
#define UART_MIDI_CHANNEL               1

///
/// \brief Defines UART channel used for communication with USB link.
///
#define UART_USB_LINK_CHANNEL           0

///
/// \brief Constant used to debounce button readings.
///
#define BUTTON_DEBOUNCE_COMPARE         0b11110000

///
/// \brief Total number of analog multiplexers.
///
#define NUMBER_OF_MUX                   7

///
/// \brief Total number of inputs on single analog multiplexer.
///
#define NUMBER_OF_MUX_INPUTS            16

///
/// \brief Total number of LED columns in LED matrix.
///
#define NUMBER_OF_LED_COLUMNS           8

///
/// \brief Total number of LED rows in LED matrix.
///
#define NUMBER_OF_LED_ROWS              6

///
/// \brief Total number of button columns in button matrix.
///
#define NUMBER_OF_BUTTON_COLUMNS        8

///
/// \brief Total number of button rows in button matrix.
///
#define NUMBER_OF_BUTTON_ROWS           6

///
/// \brief Indicates that the board supports LEDs.
///
#define LEDS_SUPPORTED

///
/// \brief LED fading is supported.
///
#define LED_FADING

///
/// \brief Use inverted logic when controlling external LEDs (high/off, low/on).
///
#define LED_EXT_INVERT

///
/// brief Total number of analog components.
///
#define MAX_NUMBER_OF_ANALOG            (NUMBER_OF_MUX*NUMBER_OF_MUX_INPUTS)

///
/// \brief Maximum number of buttons.
///
#define MAX_NUMBER_OF_BUTTONS           (NUMBER_OF_BUTTON_COLUMNS*NUMBER_OF_BUTTON_ROWS)

///
/// \brief Maximum number of encoders.
/// Total number of encoders is total number of buttons divided by two.
///
#define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)

///
/// \brief Maximum number of LEDs.
///
#define MAX_NUMBER_OF_LEDS              (NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS)

///
/// \brief Maximum number of RGB LEDs.
/// One RGB LED requires three standard LED connections.
///
#define MAX_NUMBER_OF_RGB_LEDS          (MAX_NUMBER_OF_LEDS/3)

///
/// \brief If touchscreen isn't supported, set MAX_TOUCHSCREEN_BUTTONS to zero.
///
#define MAX_TOUCHSCREEN_BUTTONS         0

///
/// \brief Indicates that the board supports display interface.
///
#define DISPLAY_SUPPORTED

///
/// \brief Maximum number of supported touchscreen buttons.
///
#define MAX_TOUCHSCREEN_BUTTONS         0
