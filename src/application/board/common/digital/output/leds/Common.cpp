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

#include <util/atomic.h>
#include "board/common/digital/output/Variables.h"
#include "board/common/constants/LEDs.h"

namespace Board
{
    namespace detail
    {
        volatile uint8_t    pwmSteps;
        volatile int8_t     transitionCounter[MAX_NUMBER_OF_LEDS];
    }

    ///
    /// \brief Array holding current LED status for all LEDs.
    ///
    uint8_t             ledState[MAX_NUMBER_OF_LEDS];

    bool setLEDfadeSpeed(uint8_t transitionSpeed)
    {
        using namespace Board::detail;

        if (transitionSpeed > FADE_TIME_MAX)
        {
            return false;
        }

        //reset transition counter
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
                transitionCounter[i] = 0;

            pwmSteps = transitionSpeed;
        }

        return true;
    }
}