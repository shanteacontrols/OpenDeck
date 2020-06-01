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
/// \brief Indicates that the board supports USB MIDI.
///
#define USB_MIDI_SUPPORTED

///
/// \brief Defines total number of available UART interfaces on board.
///
#define UART_INTERFACES                 1

///
/// \brief Total number of analog multiplexers.
///
#define NUMBER_OF_MUX                   2

///
/// \brief Total number of inputs on single analog multiplexer.
///
#define NUMBER_OF_MUX_INPUTS            16

///
/// brief Total number of analog components.
///
#define MAX_NUMBER_OF_ANALOG            (NUMBER_OF_MUX*NUMBER_OF_MUX_INPUTS)

///
/// \brief Maximum number of LEDs.
///
#define MAX_NUMBER_OF_LEDS              0

///
/// \brief Maximum number of RGB LEDs.
/// One RGB LED requires three standard LED connections.
///
#define MAX_NUMBER_OF_RGB_LEDS          0

///
/// \brief Maximum number of buttons.
///
#define MAX_NUMBER_OF_BUTTONS           14

///
/// \brief Maximum number of encoders.
/// Total number of encoders is total number of buttons divided by two.
///
#define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)

///
/// \brief Maximum number of supported touchscreen buttons.
///
#define MAX_TOUCHSCREEN_BUTTONS         80

///
/// \brief Specifies resolution of the ADC used on board.
///
#define ADC_12_BIT

///
/// \brief Indicates that the board supports touchscreen interface.
///
#define TOUCHSCREEN_SUPPORTED

///
/// \brief Defines UART channel used for touchscreen.
///
#define UART_TOUCHSCREEN_CHANNEL 0