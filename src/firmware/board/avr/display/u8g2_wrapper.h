/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

//c++ wrapper based on original wrapper for arduino

#include "../../Board.h"
#include "u8g2/csrc/u8x8.h"
#include "../../../database/blocks/Display.h"

class U8X8
{
    public:
    uint8_t tx, ty;

    U8X8();
    void setupController(displayController controller);
    void setupResolution(displayResolution resolution);
    void initDisplay();
    void setI2CAddress(uint8_t adr);
    uint8_t getCols();
    uint8_t getRows();
    void drawTile(uint8_t x, uint8_t y, uint8_t cnt, uint8_t *tile_ptr);
    void clearDisplay();
    void fillDisplay();
    void setPowerSave(uint8_t is_enable);
    void begin();
    void setFlipMode(uint8_t mode);
    void refreshDisplay(void);
    void clearLine(uint8_t line);
    void setContrast(uint8_t value);
    void setInverseFont(uint8_t value);
    void setFont(const uint8_t *font_8x8);
    void drawGlyph(uint8_t x, uint8_t y, uint8_t encoding);
    void draw2x2Glyph(uint8_t x, uint8_t y, uint8_t encoding);
    void drawString(uint8_t x, uint8_t y, const char *s);
    void drawUTF8(uint8_t x, uint8_t y, const char *s);
    void draw2x2String(uint8_t x, uint8_t y, const char *s);
    void draw2x2UTF8(uint8_t x, uint8_t y, const char *s);
    uint8_t getUTF8Len(const char *s);
    void inverse();
    void noInverse();
    uint8_t getMenuEvent();
    uint8_t userInterfaceSelectionList(const char *title, uint8_t start_pos, const char *sl);
    uint8_t userInterfaceMessage(const char *title1, const char *title2, const char *title3, const char *buttons);
    uint8_t userInterfaceInputValue(const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post);

     /* LiquidCrystal compatible functions */
    void home();
    void clear();
    void noDisplay();
    void display();
    void setCursor(uint8_t x, uint8_t y);

    protected:
    u8x8_t u8x8;

    private:
    displayResolution resolution;
    displayController controller;
};
