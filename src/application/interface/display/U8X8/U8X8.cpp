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

#include "U8X8.h"
#include "i2c/i2cmaster.h"

namespace
{
    u8x8_t u8x8;
    uint8_t rows, columns;
}

namespace U8X8
{
    bool initDisplay(displayController_t controller, displayResolution_t resolution)
    {
        bool success = false;

        //setup defaults
        u8x8_SetupDefaults(&u8x8);

        //i2c hw access
        auto gpioDelay = [](u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void *arg_ptr) -> uint8_t
        {
            return 0;
        };

        auto i2cHWA = [](u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) -> uint8_t
        {
            uint8_t *array = (uint8_t *)arg_ptr;

            switch(msg)
            {
                case U8X8_MSG_BYTE_SEND:
                for (int i=0; i<arg_int; i++)
                    i2c_write(array[i]);
                break;

                case U8X8_MSG_BYTE_INIT:
                i2c_init();
                break;

                case U8X8_MSG_BYTE_START_TRANSFER:
                i2c_start_wait(u8x8_GetI2CAddress(u8x8) + I2C_WRITE);
                break;

                case U8X8_MSG_BYTE_END_TRANSFER:
                i2c_stop();
                break;

                default:
                return 0;
            }

            return 1;
        };

        //setup specific callbacks depending on controller/resolution
        if ((resolution == displayRes_128x64) && (controller == displayController_ssd1306))
        {
            u8x8.display_cb = u8x8_d_ssd1306_128x64_noname;
            u8x8.cad_cb = u8x8_cad_ssd13xx_i2c;
            u8x8.byte_cb = i2cHWA;
            u8x8.gpio_and_delay_cb = gpioDelay;
            rows = 4;
            columns = 16;
            success = true;
        }
        else if ((resolution == displayRes_128x32) && (controller == displayController_ssd1306))
        {
            u8x8.display_cb = u8x8_d_ssd1306_128x32_univision;
            u8x8.cad_cb = u8x8_cad_ssd13xx_i2c;
            u8x8.byte_cb = i2cHWA;
            u8x8.gpio_and_delay_cb = gpioDelay;
            rows = 2;
            columns = 16;
            success = true;
        }

        if (success)
        {
            /* setup display info */
            u8x8_SetupMemory(&u8x8);
            u8x8_InitDisplay(&u8x8);

            clearDisplay();
            setPowerSave(0);

            return true;
        }

        return false;
    }

    uint8_t getColumns()
    {
        return columns;
    }

    uint8_t getRows()
    {
        return rows;
    }

    void clearDisplay()
    {
        u8x8_ClearDisplay(&u8x8);
    }

    void setPowerSave(uint8_t is_enable)
    {
        u8x8_SetPowerSave(&u8x8, is_enable);
    }

    void setFlipMode(uint8_t mode)
    {
        u8x8_SetFlipMode(&u8x8, mode);
    }

    void setFont(const uint8_t *font_8x8)
    {
        u8x8_SetFont(&u8x8, font_8x8);
    }

    void drawGlyph(uint8_t x, uint8_t y, uint8_t encoding)
    {
        u8x8_DrawGlyph(&u8x8, x, y, encoding);
    }
}