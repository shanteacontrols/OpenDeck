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

#include <util/atomic.h>
#include "board/Board.h"
#include "Variables.h"
#include "core/src/general/BitManipulation.h"
#include "core/src/HAL/avr/adc/ADC.h"

namespace Board
{
    namespace detail
    {
        uint8_t             analogIndex;
        volatile uint8_t    analogSampleCounter;
        volatile int16_t    analogBuffer[MAX_NUMBER_OF_ANALOG];
    }

    namespace
    {
        ///
        /// \brief List of all possible hysteresis regions.
        ///
        enum class hysteresisType_t: uint8_t
        {
            lowHysteresis,
            highHysteresis
        };

        ///
        /// Due to non-linearity of standard potentiometers on their extremes (low and high values),
        /// hysteresis is used to avoid incorrect values. These arrays hold information on whether
        /// low or high hysteresis values should be used.
        /// @{

        uint8_t      lowHysteresisActive[MAX_NUMBER_OF_ANALOG];
        uint8_t      highHysteresisActive[MAX_NUMBER_OF_ANALOG];

        /// @}

        ///
        /// brief Checks if specified hysteresis is active for requested analog index.
        /// @param[in] type     Hysteresis type. Enumerated type (see hysteresisType_t).
        /// @param[in] analogID Analog index for which hysteresis state is being checked.
        /// \returns True if hysteresis is currently active, false otherwise.
        ///
        bool isHysteresisActive(hysteresisType_t type, uint8_t analogID)
        {
            if (type == hysteresisType_t::lowHysteresis)
                return lowHysteresisActive[analogID];
            else
                return highHysteresisActive[analogID];
        }

        ///
        /// brief Enables or disables specific hysteresis type for specified analog index.
        /// @param[in] type     Hysteresis type. Enumerated type (see hysteresisType_t).
        /// @param[in] analogID Analog index for which hysteresis state is being changed.
        /// @param[in] state    New hystersis state (true/enabled, false/disabled).
        ///
        void updateHysteresisState(hysteresisType_t type, uint8_t analogID, bool state)
        {
            if (type == hysteresisType_t::lowHysteresis)
                lowHysteresisActive[analogID] = state;
            else
                highHysteresisActive[analogID] = state;
        }
    }

    int16_t getAnalogValue(uint8_t analogID)
    {
        using namespace Board::detail;

        int16_t value;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            value = analogBuffer[analogID] >> ANALOG_SAMPLE_SHIFT;
            analogBuffer[analogID] = 0;
        }

        if (value > HYSTERESIS_THRESHOLD_HIGH)
        {
            updateHysteresisState(hysteresisType_t::highHysteresis, analogID, true);
            updateHysteresisState(hysteresisType_t::lowHysteresis, analogID, false);

            value += HYSTERESIS_ADDITION;

            if (value > ADC_MAX_VALUE)
                return ADC_MAX_VALUE;

            return value;
        }
        else
        {
            if (value < (HYSTERESIS_THRESHOLD_HIGH - HYSTERESIS_ADDITION))
            {
                //value is now either in non-hysteresis area or low hysteresis area

                updateHysteresisState(hysteresisType_t::highHysteresis, analogID, false);

                if (value < (HYSTERESIS_THRESHOLD_LOW + HYSTERESIS_SUBTRACTION))
                {
                    if (value < HYSTERESIS_THRESHOLD_LOW)
                    {
                        updateHysteresisState(hysteresisType_t::lowHysteresis, analogID, true);
                        value -= HYSTERESIS_SUBTRACTION;

                        if (value < 0)
                            value = 0;

                        return value;
                    }
                    else
                    {
                        if (isHysteresisActive(hysteresisType_t::lowHysteresis, analogID))
                        {
                            value -= HYSTERESIS_SUBTRACTION;

                            if (value < 0)
                                return 0;
                        }

                        return value;
                    }
                }

                updateHysteresisState(hysteresisType_t::lowHysteresis, analogID, false);
                updateHysteresisState(hysteresisType_t::highHysteresis, analogID, false);

                return value;
            }
            else
            {
                if (isHysteresisActive(hysteresisType_t::highHysteresis, analogID))
                {
                    //high hysteresis still enabled
                    value += HYSTERESIS_ADDITION;

                    if (value > ADC_MAX_VALUE)
                        return ADC_MAX_VALUE;

                    return value;
                }
                else
                {
                    updateHysteresisState(hysteresisType_t::highHysteresis, analogID, false);
                    return value;
                }
            }
        }
    }

    bool analogDataAvailable()
    {
        using namespace Board::detail;
        return (analogSampleCounter == NUMBER_OF_ANALOG_SAMPLES);
    }

    void continueADCreadout()
    {
        using namespace Board::detail;
        analogSampleCounter = 0;
        analogIndex = 0;
        startADCconversion();
    }
}