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

#include <stdlib.h>
#include "Common.h"
#include "board/common/constants/Encoders.h"

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Array holding last two readings from encoder pins.
        ///
        uint8_t encoderData[MAX_NUMBER_OF_ENCODERS];

        ///
        /// \brief Array holding current amount of pulses for all encoders.
        ///
        int8_t  encoderPulses[MAX_NUMBER_OF_ENCODERS];

        ///
        /// \brief Array holding last encoder direction.
        ///
        bool    lastEncoderDirection[MAX_NUMBER_OF_ENCODERS];

        ///
        /// \brief Checks state of requested encoder.
        /// Internal function.
        /// @param [in] encoderID       Encoder which is being checked.
        /// @param [in] pairState       A and B signal readings from encoder placed into bits 0 and 1.
        /// @param [in] pulsesPerStep   Amount of pulses per encoder step.
        /// \returns Encoder direction. See encoderPosition_t.
        ///
        encoderPosition_t readEncoder(uint8_t encoderID, uint8_t pairState, uint8_t pulsesPerStep)
        {
            using namespace Board::detail;

            encoderPosition_t returnValue = encStopped;

            //add new data
            encoderData[encoderID] <<= 2;
            encoderData[encoderID] |= pairState;
            encoderData[encoderID] &= 0x0F;

            //no point in further checks if no movement is detected
            if (!encoderLookUpTable[encoderData[encoderID]])
                return returnValue;

            encoderPulses[encoderID] += encoderLookUpTable[encoderData[encoderID]];

            bool newEncoderDirection = encoderLookUpTable[encoderData[encoderID]] > 0;

            //get last encoder direction
            bool lastDirection = lastEncoderDirection[encoderID];

            //update new direction
            lastEncoderDirection[encoderID] = newEncoderDirection;

            //in order to detect single step, all pulse readings must have same direction
            if (lastDirection != newEncoderDirection)
            {
                //reset encoder pulses to current lookup value
                encoderPulses[encoderID] = encoderLookUpTable[encoderData[encoderID]];
            }

            if (abs(encoderPulses[encoderID]) >= pulsesPerStep)
            {
                returnValue = (encoderPulses[encoderID] > 0) ? encMoveLeft : encMoveRight;
                //reset count
                encoderPulses[encoderID] = 0;
            }

            return returnValue;
        }
    }
}