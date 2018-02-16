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

#include "u8g2_wrapper.h"
#include "i2c/i2cmaster.h"

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
    return 0;
}

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    // uint8_t *array = (uint8_t *)arg_ptr;

    switch(msg)
    {
        case U8X8_MSG_BYTE_SEND:
        for (int i=0; i<arg_int; i++)
        {
            i2c_write(array[i]);
        }
        break;

        case U8X8_MSG_BYTE_INIT:
        i2c_init();
        break;

        case U8X8_MSG_BYTE_SET_DC:
        break;

        case U8X8_MSG_BYTE_START_TRANSFER:
        i2c_start_wait((u8x8_GetI2CAddress(u8x8) >> 1) + I2C_WRITE);
        break;

        case U8X8_MSG_BYTE_END_TRANSFER:
        i2c_stop();
        break;

        default:
        return 0;
    }

    return 1;
}

U8X8::U8X8()
{

}

void U8X8::initDisplay(displayController_t controller, displayResolution_t resolution)
{
    //setup defaults
    u8x8_SetupDefaults(&u8x8);

    //setup specific callbacks depending on controller/resolution
    if ((resolution == displayRes_128x64) && (controller == displayController_ssd1306))
    {
        u8x8.display_cb = u8x8_d_ssd1306_128x64_noname;
        u8x8.cad_cb = u8x8_cad_ssd13xx_i2c;
        u8x8.byte_cb = u8x8_byte_hw_i2c;
        u8x8.gpio_and_delay_cb = u8x8_gpio_and_delay;
    }

    /* setup display info */
    u8x8_SetupMemory(&u8x8);
    u8x8_InitDisplay(&u8x8);

    clearDisplay();
    setPowerSave(0);
}

void U8X8::setI2CAddress(uint8_t adr)
{
    u8x8_SetI2CAddress(&u8x8, adr);
}

uint8_t U8X8::getCols()
{
    return u8x8_GetCols(&u8x8);
}

uint8_t U8X8::getRows()
{
    return u8x8_GetRows(&u8x8);
}

void U8X8::drawTile(uint8_t x, uint8_t y, uint8_t cnt, uint8_t *tile_ptr)
{
    u8x8_DrawTile(&u8x8, x, y, cnt, tile_ptr);
}

void U8X8::clearDisplay()
{
    u8x8_ClearDisplay(&u8x8);
}

void U8X8::fillDisplay()
{
    u8x8_FillDisplay(&u8x8);
}

void U8X8::setPowerSave(uint8_t is_enable)
{
    u8x8_SetPowerSave(&u8x8, is_enable);
}

void U8X8::setFlipMode(uint8_t mode)
{
    u8x8_SetFlipMode(&u8x8, mode);
}

void U8X8::refreshDisplay(void)
{
    // Dec 16: Only required for SSD1606
    u8x8_RefreshDisplay(&u8x8);
}

void U8X8::clearLine(uint8_t line)
{
    u8x8_ClearLine(&u8x8, line);
}

void U8X8::setContrast(uint8_t value)
{
    u8x8_SetContrast(&u8x8, value);
}

void U8X8::setInverseFont(uint8_t value)
{
    u8x8_SetInverseFont(&u8x8, value);
}

void U8X8::setFont(const uint8_t *font_8x8)
{
    u8x8_SetFont(&u8x8, font_8x8);
}

void U8X8::drawGlyph(uint8_t x, uint8_t y, uint8_t encoding)
{
    u8x8_DrawGlyph(&u8x8, x, y, encoding);
}

void U8X8::draw2x2Glyph(uint8_t x, uint8_t y, uint8_t encoding)
{
    u8x8_Draw2x2Glyph(&u8x8, x, y, encoding);
}

void U8X8::drawString(uint8_t x, uint8_t y, const char *s)
{
    u8x8_DrawString(&u8x8, x, y, s);
}

void U8X8::drawUTF8(uint8_t x, uint8_t y, const char *s)
{
    u8x8_DrawUTF8(&u8x8, x, y, s);
}

void U8X8::draw2x2String(uint8_t x, uint8_t y, const char *s)
{
    u8x8_Draw2x2String(&u8x8, x, y, s);
}

void U8X8::draw2x2UTF8(uint8_t x, uint8_t y, const char *s)
{
    u8x8_Draw2x2UTF8(&u8x8, x, y, s);
}

uint8_t U8X8::getUTF8Len(const char *s)
{
    return u8x8_GetUTF8Len(&u8x8, s);
}

void U8X8::inverse()
{
    setInverseFont(1);
}

void U8X8::noInverse()
{
    setInverseFont(0);
}

/* return 0 for no event or U8X8_MSG_GPIO_MENU_SELECT, */
/* U8X8_MSG_GPIO_MENU_NEXT, U8X8_MSG_GPIO_MENU_PREV, */
/* U8X8_MSG_GPIO_MENU_HOME */
uint8_t U8X8::getMenuEvent()
{
    return u8x8_GetMenuEvent(&u8x8);
}

uint8_t U8X8::userInterfaceSelectionList(const char *title, uint8_t start_pos, const char *sl)
{
    return u8x8_UserInterfaceSelectionList(&u8x8, title, start_pos, sl);
}

uint8_t U8X8::userInterfaceMessage(const char *title1, const char *title2, const char *title3, const char *buttons)
{
    return u8x8_UserInterfaceMessage(&u8x8, title1, title2, title3, buttons);
}

uint8_t U8X8::userInterfaceInputValue(const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post)
{
    return u8x8_UserInterfaceInputValue(&u8x8, title, pre, value, lo, hi, digits, post);
}

void U8X8::home()
{
    tx = 0;
    ty = 0;
}

void U8X8::clear()
{
    clearDisplay();
    home();
}

void U8X8::noDisplay()
{
    u8x8_SetPowerSave(&u8x8, 1);
}

void U8X8::display()
{
    u8x8_SetPowerSave(&u8x8, 0);
}

void U8X8::setCursor(uint8_t x, uint8_t y)
{
    tx = x;
    ty = y;
}

U8X8 display_hw;