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

#include <avr/io.h>

namespace Board
{
    namespace detail
    {
        #define SR_DIN_DATA_PORT            PORTD
        #define SR_DIN_DATA_PIN             3

        #define SR_DIN_CLK_PORT             PORTD
        #define SR_DIN_CLK_PIN              4

        #define SR_DIN_LATCH_PORT           PORTD
        #define SR_DIN_LATCH_PIN            5

        #define SR_OUT_DATA_PORT            PORTD
        #define SR_OUT_DATA_PIN             2

        #define SR_OUT_CLK_PORT             PORTD
        #define SR_OUT_CLK_PIN              0

        #define SR_OUT_LATCH_PORT           PORTD
        #define SR_OUT_LATCH_PIN            1

        #define MUX_S0_PORT                 PORTB
        #define MUX_S0_PIN                  3

        #define MUX_S1_PORT                 PORTB
        #define MUX_S1_PIN                  1

        #define MUX_S2_PORT                 PORTE
        #define MUX_S2_PIN                  6

        #define MUX_S3_PORT                 PORTB
        #define MUX_S3_PIN                  2

        #define MUX_1_IN_PORT               PORTF
        #define MUX_1_IN_PIN                5

        #define MUX_2_IN_PORT               PORTF
        #define MUX_2_IN_PIN                6

        #define MUX_3_IN_PORT               PORTF
        #define MUX_3_IN_PIN                7

        #define MUX_4_IN_PORT               PORTB
        #define MUX_4_IN_PIN                6

        #define BTLDR_BUTTON_INDEX          0x03
    }
}