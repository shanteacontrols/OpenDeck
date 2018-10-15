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

#include "core/src/HAL/avr/PinManipulation.h"

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Time in milliseconds during which MIDI event indicators on board are on when MIDI event happens.
        ///
        #define MIDI_INDICATOR_TIMEOUT      50

        ///
        /// \brief Time in milliseconds for single startup animation cycle on built-in LED indicators.
        ///
        #define LED_INDICATOR_STARTUP_DELAY 150
    }

    ///
    /// \brief Helper macros used for easier control of internal (on-board) and external LEDs.
    /// @{

    #ifdef LED_INT_INVERT
    #define INT_LED_ON(port, pin)       setLow(port, pin)
    #define INT_LED_OFF(port, pin)      setHigh(port, pin)
    #else
    #define INT_LED_ON(port, pin)       setHigh(port, pin)
    #define INT_LED_OFF(port, pin)      setLow(port, pin)
    #endif

    #ifdef LED_EXT_INVERT
    #define EXT_LED_ON(port, pin)       setLow(port, pin)
    #define EXT_LED_OFF(port, pin)      setHigh(port, pin)
    #else
    #define EXT_LED_ON(port, pin)       setHigh(port, pin)
    #define EXT_LED_OFF(port, pin)      setLow(port, pin)
    #endif

    /// @}

    #define FADE_TIME_MIN               0
    #define FADE_TIME_MAX               10
}