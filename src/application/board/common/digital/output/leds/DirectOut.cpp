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

#include <inttypes.h>
#include "interface/digital/output/leds/DataTypes.h"

namespace Board
{
    uint8_t getRGBaddress(uint8_t rgbID, rgbIndex_t index)
    {
        return rgbID*3+(uint8_t)index;
    }

    uint8_t getRGBID(uint8_t ledID)
    {
        uint8_t result = ledID / 3;

        if (result >= MAX_NUMBER_OF_RGB_LEDS)
            return MAX_NUMBER_OF_RGB_LEDS-1;
        else
            return result;
    }
}