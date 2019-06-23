/*

Copyright 2015-2019 Igor Petrovic

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

#ifdef __AVR__
#include "core/src/avr/ADC.h"
#endif
#include "core/src/general/BitManipulation.h"
#include "board/Board.h"
#include "board/common/Map.h"
#include "Pins.h"
#include "board/common/analog/Analog.h"

namespace
{
    uint8_t          analogIndex;
    volatile uint8_t analogSampleCounter;
    volatile int16_t analogBuffer[MAX_NUMBER_OF_ANALOG];
#ifdef NUMBER_OF_MUX
    uint8_t activeMux;
    uint8_t activeMuxInput;
#endif
}    // namespace

namespace Board
{
    namespace interface
    {
        namespace analog
        {
            int16_t readValue(uint8_t analogID)
            {
                int16_t value;

#ifdef __AVR__
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#endif
                {
                    value = analogBuffer[analogID] >> ANALOG_SAMPLE_SHIFT;
                    analogBuffer[analogID] = 0;
                }

                return value;
            }

            bool isDataAvailable()
            {
                return (analogSampleCounter == NUMBER_OF_ANALOG_SAMPLES);
            }

            void continueReadout()
            {
                analogSampleCounter = 0;
                analogIndex = 0;
                core::CORE_ARCH::adc::startConversion();
            }

            namespace detail
            {
#ifdef NUMBER_OF_MUX
                ///
                /// \brief Configures one of 16 inputs/outputs on 4067 multiplexer.
                ///
                inline void setMuxInput()
                {
                    BIT_READ(Board::map::muxChannel(activeMuxInput), 0) ? CORE_AVR_PIN_SET_HIGH(MUX_S0_PORT, MUX_S0_PIN) : CORE_AVR_PIN_SET_LOW(MUX_S0_PORT, MUX_S0_PIN);
                    BIT_READ(Board::map::muxChannel(activeMuxInput), 1) ? CORE_AVR_PIN_SET_HIGH(MUX_S1_PORT, MUX_S1_PIN) : CORE_AVR_PIN_SET_LOW(MUX_S1_PORT, MUX_S1_PIN);
                    BIT_READ(Board::map::muxChannel(activeMuxInput), 2) ? CORE_AVR_PIN_SET_HIGH(MUX_S2_PORT, MUX_S2_PIN) : CORE_AVR_PIN_SET_LOW(MUX_S2_PORT, MUX_S2_PIN);
                    BIT_READ(Board::map::muxChannel(activeMuxInput), 3) ? CORE_AVR_PIN_SET_HIGH(MUX_S3_PORT, MUX_S3_PIN) : CORE_AVR_PIN_SET_LOW(MUX_S3_PORT, MUX_S3_PIN);
                }
#endif

                void update()
                {
                    //always ignore first reading
                    static bool ignoreFirst = true;

                    if (!ignoreFirst)
                    {
                        analogBuffer[analogIndex] += ADC;
                        analogIndex++;
#ifdef NUMBER_OF_MUX
                        activeMuxInput++;

                        bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);

                        if (switchMux)
#else
                        if (analogIndex == MAX_NUMBER_OF_ANALOG)
#endif
                        {
#ifdef NUMBER_OF_MUX
                            activeMuxInput = 0;
                            activeMux++;

                            if (activeMux == NUMBER_OF_MUX)
                            {
                                activeMux = 0;
#endif
                                analogIndex = 0;
                                analogSampleCounter++;
#ifdef NUMBER_OF_MUX
                            }
#endif

#ifdef NUMBER_OF_MUX
                            //switch to next mux once all mux inputs are read
                            core::CORE_ARCH::adc::setChannel(Board::map::adcChannel(activeMux));
#endif
                        }

//always switch to next read pin
#ifdef NUMBER_OF_MUX
                        setMuxInput();
#else
                        core::CORE_ARCH::adc::setChannel(Board::map::adcChannel(analogIndex));
#endif
                    }

                    ignoreFirst = !ignoreFirst;

                    if (analogSampleCounter != NUMBER_OF_ANALOG_SAMPLES)
                        core::CORE_ARCH::adc::startConversion();
                }
            }    // namespace detail
        }        // namespace analog
    }            // namespace interface
}    // namespace Board