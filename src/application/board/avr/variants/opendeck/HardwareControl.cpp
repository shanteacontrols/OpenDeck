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
#include "../../../../interface/digital/output/leds/Variables.h"
#include "../../../../interface/digital/output/leds/Helpers.h"
#include "board/common/constants/LEDs.h"
#include "board/common/analog/input/Variables.h"
#include "board/common/digital/input/Variables.h"
#include "board/common/digital/output/Variables.h"


///
/// \brief Activates currently active button matrix column (stored in activeInColumn variable).
///
inline void activateInputColumn()
{
    BIT_READ(dmColumnArray[activeInColumn], 0) ? setHigh(DEC_DM_A0_PORT, DEC_DM_A0_PIN) : setLow(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    BIT_READ(dmColumnArray[activeInColumn], 1) ? setHigh(DEC_DM_A1_PORT, DEC_DM_A1_PIN) : setLow(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    BIT_READ(dmColumnArray[activeInColumn], 2) ? setHigh(DEC_DM_A2_PORT, DEC_DM_A2_PIN) : setLow(DEC_DM_A2_PORT, DEC_DM_A2_PIN);

    if (++activeInColumn == NUMBER_OF_BUTTON_COLUMNS)
        activeInColumn = 0;
}

///
/// \brief Switches to next LED matrix column.
///
inline void activateOutputColumn()
{
    BIT_READ(activeOutColumn, 0) ? setHigh(DEC_LM_A0_PORT, DEC_LM_A0_PIN) : setLow(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
    BIT_READ(activeOutColumn, 1) ? setHigh(DEC_LM_A1_PORT, DEC_LM_A1_PIN) : setLow(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
    BIT_READ(activeOutColumn, 2) ? setHigh(DEC_LM_A2_PORT, DEC_LM_A2_PIN) : setLow(DEC_LM_A2_PORT, DEC_LM_A2_PIN);
}

///
/// Acquires data for all buttons connected in currently active button matrix column by
/// reading inputs from shift register.
///
inline void storeDigitalIn()
{
    for (int i=0; i<NUMBER_OF_BUTTON_COLUMNS; i++)
    {
        activateInputColumn();
        _NOP();

        setLow(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
        setLow(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);
        _NOP();

        setHigh(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

        for (int j=0; j<8; j++)
        {
            setLow(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
            _NOP();
            BIT_WRITE(digitalInBuffer[dIn_head][i], dmRowBitArray[j], !readPin(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN));
            setHigh(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
        }
    }
}

///
/// \brief Turns all rows in LED matrix off.
///
inline void ledRowsOff()
{
    //turn off PWM
    TCCR1A &= ~(1<<COM1C1);
    TCCR4C &= ~(1<<COM4D1);
    TCCR1A &= ~(1<<COM1A1);
    TCCR4A &= ~(1<<COM4A1);
    TCCR3A &= ~(1<<COM3A1);
    TCCR1A &= ~(1<<COM1B1);

    for (int i=0; i<NUMBER_OF_LED_ROWS; i++)
        EXT_LED_OFF(*ledRowPins[i].port, ledRowPins[i].pin);
}

///
/// \brief Turns requested LED row on with specified PWM intensity.
/// @param [in] rowNumber   Row in LED matrix which is being turned on.
/// @param [in] intensity   PWM intensity of requested row.
///
inline void ledRowOn(uint8_t rowNumber, uint8_t intensity)
{
    if (intensity == 255)
    {
        //max value, don't use pwm
        EXT_LED_ON(*ledRowPins[rowNumber].port, ledRowPins[rowNumber].pin);
    }
    else
    {
        #ifdef LED_EXT_INVERT
        intensity = 255 - intensity;
        #endif

        switch (rowNumber)
        {
            //turn off pwm if intensity is max
            case 0:
            OCR1C = intensity;
            TCCR1A |= (1<<COM1C1);
            break;

            case 1:
            OCR4D = intensity;
            TCCR4C |= (1<<COM4D1);
            break;

            case 2:
            OCR1A = intensity;
            TCCR1A |= (1<<COM1A1);
            break;

            case 3:
            OCR4A = intensity;
            TCCR4A |= (1<<COM4A1);
            break;

            case 4:
            OCR3A = intensity;
            TCCR3A |= (1<<COM3A1);
            break;

            case 5:
            OCR1B = intensity;
            TCCR1A |= (1<<COM1B1);
            break;

            default:
            break;
        }
    }
}

///
/// \brief Checks if any of LEDs in currently active LED matrix column needs to be turned on.
/// This function also performs transition effect betweeen two LED states.
///
inline void checkLEDs()
{
    ledRowsOff();
    activateOutputColumn();

    //if there is an active LED in current column, turn on LED row
    //do fancy transitions here
    for (int i=0; i<NUMBER_OF_LED_ROWS; i++)
    {
        uint8_t ledNumber = activeOutColumn+i*NUMBER_OF_LED_COLUMNS;
        uint8_t ledStateSingle = LED_ON(ledState[ledNumber]);

        ledStateSingle *= (NUMBER_OF_LED_TRANSITIONS-1);

        //don't bother with pwm if it's disabled
        if (!pwmSteps && ledStateSingle)
        {
            ledRowOn(i, 255);
        }
        else
        {
            if (ledTransitionScale[transitionCounter[ledNumber]])
                ledRowOn(i, ledTransitionScale[transitionCounter[ledNumber]]);

            if (transitionCounter[ledNumber] != ledStateSingle)
            {
                //fade up
                if (transitionCounter[ledNumber] < ledStateSingle)
                {
                    transitionCounter[ledNumber] += pwmSteps;

                    if (transitionCounter[ledNumber] > ledStateSingle)
                        transitionCounter[ledNumber] = ledStateSingle;
                }
                else
                {
                    //fade down
                    transitionCounter[ledNumber] -= pwmSteps;

                    if (transitionCounter[ledNumber] < 0)
                        transitionCounter[ledNumber] = 0;
                }
            }
        }
    }

    if (++activeOutColumn == NUMBER_OF_LED_COLUMNS)
        activeOutColumn = 0;
}

///
/// \brief Configures one of 16 inputs/outputs on 4067 multiplexer.
///
inline void setMuxInput()
{
    BIT_READ(muxPinOrderArray[activeMuxInput], 0) ? setHigh(MUX_S0_PORT, MUX_S0_PIN) : setLow(MUX_S0_PORT, MUX_S0_PIN);
    BIT_READ(muxPinOrderArray[activeMuxInput], 1) ? setHigh(MUX_S1_PORT, MUX_S1_PIN) : setLow(MUX_S1_PORT, MUX_S1_PIN);
    BIT_READ(muxPinOrderArray[activeMuxInput], 2) ? setHigh(MUX_S2_PORT, MUX_S2_PIN) : setLow(MUX_S2_PORT, MUX_S2_PIN);
    BIT_READ(muxPinOrderArray[activeMuxInput], 3) ? setHigh(MUX_S3_PORT, MUX_S3_PIN) : setLow(MUX_S3_PORT, MUX_S3_PIN);
}
