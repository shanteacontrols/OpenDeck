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