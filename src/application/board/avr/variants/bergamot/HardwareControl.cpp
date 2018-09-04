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

#include "pins/Map.h"
#include "board/common/analog/input/Variables.h"
#include "board/common/digital/input/Variables.h"


///
/// Acquires data by reading all inputs from connected shift register.
///
inline void storeDigitalIn()
{
    setLow(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
    setLow(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);
    _NOP();

    setHigh(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

    for (int i=0; i<NUMBER_OF_IN_SR_INPUTS; i++)
    {
        setLow(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
        _NOP();
        BIT_WRITE(digitalInBuffer[dIn_head][0], 7-i, !readPin(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN));
        setHigh(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
    }
}

///
/// \brief Configures one of 16 inputs/outputs on 4067 multiplexer.
///
inline void setMuxInput()
{
    BIT_READ(activeMuxInput, 0) ? setHigh(MUX_S0_PORT, MUX_S0_PIN) : setLow(MUX_S0_PORT, MUX_S0_PIN);
    BIT_READ(activeMuxInput, 1) ? setHigh(MUX_S1_PORT, MUX_S1_PIN) : setLow(MUX_S1_PORT, MUX_S1_PIN);
    BIT_READ(activeMuxInput, 2) ? setHigh(MUX_S2_PORT, MUX_S2_PIN) : setLow(MUX_S2_PORT, MUX_S2_PIN);
    BIT_READ(activeMuxInput, 3) ? setHigh(MUX_S3_PORT, MUX_S3_PIN) : setLow(MUX_S3_PORT, MUX_S3_PIN);
}
