/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Constant used to debounce button readings.
        ///
        #define BUTTON_DEBOUNCE_COMPARE         0b11110000

        ///
        /// \brief Total number of analog multiplexers.
        ///
        #define NUMBER_OF_MUX                   2

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
    /// Total number of encoders is total number of buttons divided by two.
    ///
    #define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)

    ///
    /// \brief If touchscreen isn't supported, set MAX_TOUCHSCREEN_BUTTONS to zero.
    ///
    #define MAX_TOUCHSCREEN_BUTTONS         0
}