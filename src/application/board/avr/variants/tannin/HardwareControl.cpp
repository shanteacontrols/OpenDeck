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

#include "pins/Map.h"
#include "interface/digital/output/leds/Helpers.h"
#include "board/common/constants/LEDs.h"
#include "board/common/analog/input/Variables.h"
#include "board/common/digital/input/Variables.h"
#include "board/common/digital/output/Variables.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/general/BitManipulation.h"

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Activates currently active LED matrix column (stored in activeOutColumn variable).
        ///
        inline void activateOutputColumn()
        {
            BIT_READ(activeOutColumn, 0) ? setHigh(DEC_LM_A0_PORT, DEC_LM_A0_PIN) : setLow(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
            BIT_READ(activeOutColumn, 1) ? setHigh(DEC_LM_A1_PORT, DEC_LM_A1_PIN) : setLow(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
            BIT_READ(activeOutColumn, 2) ? setHigh(DEC_LM_A2_PORT, DEC_LM_A2_PIN) : setLow(DEC_LM_A2_PORT, DEC_LM_A2_PIN);
        }

        ///
        /// \brief Activates currently active button matrix column (stored in activeInColumn variable).
        ///
        inline void activateInputColumn()
        {
            BIT_READ(activeInColumn, 0) ? setHigh(DEC_DM_A0_PORT, DEC_DM_A0_PIN) : setLow(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
            BIT_READ(activeInColumn, 1) ? setHigh(DEC_DM_A1_PORT, DEC_DM_A1_PIN) : setLow(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
            BIT_READ(activeInColumn, 2) ? setHigh(DEC_DM_A2_PORT, DEC_DM_A2_PIN) : setLow(DEC_DM_A2_PORT, DEC_DM_A2_PIN);

            if (++activeInColumn == NUMBER_OF_BUTTON_COLUMNS)
                activeInColumn = 0;
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
                    BIT_WRITE(digitalInBuffer[dIn_head][i], 7-j, !readPin(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN));
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
            TCCR1A &= ~(1<<COM1A1); //oc1a
            TCCR1A &= ~(1<<COM1B1); //oc1b
            TCCR3A &= ~(1<<COM3A1); //oc3a
            TCCR4A &= ~(1<<COM4A1); //oc4a

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
                    OCR1A = intensity;
                    TCCR1A |= (1<<COM1A1);
                    break;

                    case 1:
                    OCR1B = intensity;
                    TCCR1A |= (1<<COM1B1);
                    break;

                    case 2:
                    OCR3A = intensity;
                    TCCR3A |= (1<<COM3A1);
                    break;

                    case 3:
                    OCR4A = intensity;
                    TCCR4A |= (1<<COM4A1);
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
    }
}