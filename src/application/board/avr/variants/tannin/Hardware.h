/*

Copyright 2015-2018 Igor Petrovic

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

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Constant used to debounce button readings.
        ///
        #define BUTTON_DEBOUNCE_COMPARE         0b00000000

        ///
        /// \brief Total number of analog multiplexers.
        ///
        #define NUMBER_OF_MUX                   1

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
        #define NUMBER_OF_LED_ROWS              4

        ///
        /// \brief Total number of button columns in button matrix.
        ///
        #define NUMBER_OF_BUTTON_COLUMNS        8

        ///
        /// \brief Total number of button rows in button matrix.
        ///
        #define NUMBER_OF_BUTTON_ROWS           8
    }

    ///
    /// brief Total number of analog components.
    ///
    #define MAX_NUMBER_OF_ANALOG            (NUMBER_OF_MUX*NUMBER_OF_MUX_INPUTS)

    ///
    /// \brief Maximum number of buttons.
    ///
    #define MAX_NUMBER_OF_BUTTONS           (NUMBER_OF_BUTTON_COLUMNS*NUMBER_OF_BUTTON_ROWS)

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
    /// \brief Maximum number of encoders.
    /// Since encoders are also connected in button matrix and encoders need two
    /// pins, total number of encoders is total number of buttons divided by two.
    ///
    #define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)

    ///
    /// \brief If touchscreen isn't supported, set MAX_TOUCHSCREEN_BUTTONS to zero.
    ///
    #define MAX_TOUCHSCREEN_BUTTONS         0
}