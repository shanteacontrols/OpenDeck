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

#define ENCODER_VALUE_LEFT_7FH01H   127
#define ENCODER_VALUE_RIGHT_7FH01H  1

#define ENCODER_VALUE_LEFT_3FH41H   63
#define ENCODER_VALUE_RIGHT_3FH41H  65

///
/// \brief Time in milliseconds after which debounce mode is reset if encoder isn't moving.
///
#define DEBOUNCE_RESET_TIME     50

///
/// Number of times movement in the same direction must be registered in order
/// for debouncer to become active. Once the debouncer is active, all further changes
/// in the movement will be ignored, that is, all movements will be registered in the
/// direction which was repeated. This state is reset until the encoder is either stopped
/// or if same amount of movements are registered in the opposite direction.
///
#define ENCODER_DEBOUNCE_COUNT  4

///
/// \brief Array used for easier access to current encoder MIDI value in 7Fh01h and 3Fh41h modes.
/// Matched with encoderType_t and encoderPosition_t
///
const uint8_t       encValue[2][3] =
{
    {
        0,
        ENCODER_VALUE_LEFT_7FH01H,
        ENCODER_VALUE_RIGHT_7FH01H
    },

    {
        0,
        ENCODER_VALUE_LEFT_3FH41H,
        ENCODER_VALUE_RIGHT_3FH41H
    }
};