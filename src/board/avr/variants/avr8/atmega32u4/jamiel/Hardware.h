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

#include "../MCU.h"

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
/// \brief Indicates that the board supports USB MIDI.
///
#define USB_MIDI_SUPPORTED

///
/// \brief Constant used to debounce button readings.
///
#define BUTTON_DEBOUNCE_COMPARE         0b00000000

///
/// \brief Total number of analog multiplexers.
///
#define NUMBER_OF_MUX                   3

///
/// \brief Total number of inputs on single analog multiplexer.
///
#define NUMBER_OF_MUX_INPUTS            16

///
/// \brief Total number of connected input shift register.
///
#define NUMBER_OF_IN_SR                 4

///
/// \brief Total number of inputs on single input shift register.
///
#define NUMBER_OF_IN_SR_INPUTS          8

///
/// \brief Total number of connected output shift register.
///
#define NUMBER_OF_OUT_SR                4

///
/// \brief Total number of outputs on single output shift register.
///
#define NUMBER_OF_OUT_SR_INPUTS         8

///
/// brief Total number of analog components.
///
#define MAX_NUMBER_OF_ANALOG            (NUMBER_OF_MUX*NUMBER_OF_MUX_INPUTS)

///
/// \brief Maximum number of buttons.
///
#define MAX_NUMBER_OF_BUTTONS           (NUMBER_OF_IN_SR*NUMBER_OF_IN_SR_INPUTS)

///
/// \brief Indicates that the board supports LEDs.
///
#define LEDS_SUPPORTED

///
/// \brief Maximum number of LEDs.
///
#define MAX_NUMBER_OF_LEDS              (NUMBER_OF_OUT_SR*NUMBER_OF_OUT_SR_INPUTS)

///
/// \brief Use inverted logic when controlling external LEDs (high/off, low/on).
///
#define LED_EXT_INVERT

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
/// \brief If touchscreen isn't supported, set MAX_TOUCHSCREEN_BUTTONS to zero.
///
#define MAX_TOUCHSCREEN_BUTTONS         0
