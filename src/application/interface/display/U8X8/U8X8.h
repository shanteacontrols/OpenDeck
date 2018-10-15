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

#include "u8g2/csrc/u8x8.h"
#include "database/blocks/Display.h"

namespace U8X8
{
    bool initDisplay(displayController_t controller, displayResolution_t resolution);
    uint8_t getColumns();
    uint8_t getRows();
    void clearDisplay();
    void setPowerSave(uint8_t is_enable);
    void setFlipMode(uint8_t mode);
    void setFont(const uint8_t *font_8x8);
    void drawGlyph(uint8_t x, uint8_t y, uint8_t encoding);
}