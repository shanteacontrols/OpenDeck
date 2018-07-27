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

///
/// \brief Pin definitions.
/// @{

#define SR_DIN_DATA_PORT            PORTD
#define SR_DIN_DATA_PIN             4

#define SR_DIN_CLK_PORT             PORTD
#define SR_DIN_CLK_PIN              3

#define SR_DIN_LATCH_PORT           PORTD
#define SR_DIN_LATCH_PIN            5

#define DEC_DM_A0_PORT              PORTD
#define DEC_DM_A0_PIN               0

#define DEC_DM_A1_PORT              PORTD
#define DEC_DM_A1_PIN               1

#define DEC_DM_A2_PORT              PORTD
#define DEC_DM_A2_PIN               2

#define DEC_LM_A0_PORT              PORTD
#define DEC_LM_A0_PIN               6

#define DEC_LM_A1_PORT              PORTD
#define DEC_LM_A1_PIN               7

#define DEC_LM_A2_PORT              PORTB
#define DEC_LM_A2_PIN               4

#define LED_ROW_1_PORT              PORTB
#define LED_ROW_1_PIN               5

#define LED_ROW_2_PORT              PORTB
#define LED_ROW_2_PIN               6

#define LED_ROW_3_PORT              PORTC
#define LED_ROW_3_PIN               6

#define LED_ROW_4_PORT              PORTC
#define LED_ROW_4_PIN               7

#define MUX_S0_PORT                 PORTF
#define MUX_S0_PIN                  6

#define MUX_S1_PORT                 PORTF
#define MUX_S1_PIN                  5

#define MUX_S2_PORT                 PORTF
#define MUX_S2_PIN                  7

#define MUX_S3_PORT                 PORTF
#define MUX_S3_PIN                  4

#define MUX_1_IN_PORT               PORTF
#define MUX_1_IN_PIN                1

/// @}