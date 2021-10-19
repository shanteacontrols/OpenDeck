/*

Copyright 2015-2021 Igor Petrovic

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

#include "U8X8.h"
#include <string.h>

using namespace IO;

bool U8X8::init(uint8_t i2cAddressIndex, displayController_t controller, displayResolution_t resolution)
{
    bool success = false;

    // setup defaults
    u8x8_SetupDefaults(&_u8x8);

    // i2c hw access
    auto gpioDelay = [](u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void* arg_ptr) -> uint8_t {
        return 0;
    };

    // hack: make the hwa reference static pointer so that access to hwa
    // works inside lambda
    // we are interfacing with C library!

    static HWAI2C* hwaStatic;
    hwaStatic = &_hwa;

    auto i2cHWA = [](u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) -> uint8_t {
        auto* array = (uint8_t*)arg_ptr;

        // u8x8 lib doesn't send packets larger than 32 bytes
        static uint8_t buffer[32];
        static size_t  counter = 0;

        switch (msg)
        {
        case U8X8_MSG_BYTE_SEND:
        {
            memcpy(&buffer[counter], array, arg_int);
            counter += arg_int;
        }
        break;

        case U8X8_MSG_BYTE_INIT:
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
        {
            counter = 0;
        }
        break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            return hwaStatic->write(u8x8_GetI2CAddress(u8x8), buffer, counter);

        default:
            return 0;
        }

        return 1;
    };

    // setup specific callbacks depending on controller/resolution
    if ((resolution == displayResolution_t::_128x64) && (controller == displayController_t::ssd1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x64_noname;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2cHWA;
        _u8x8.gpio_and_delay_cb = gpioDelay;
        _rows                   = 4;
        _columns                = 16;
        success                 = true;
    }
    else if ((resolution == displayResolution_t::_128x32) && (controller == displayController_t::ssd1306))
    {
        _u8x8.display_cb        = u8x8_d_ssd1306_128x32_univision;
        _u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        _u8x8.byte_cb           = i2cHWA;
        _u8x8.gpio_and_delay_cb = gpioDelay;
        _rows                   = 2;
        _columns                = 16;
        success                 = true;
    }

    uint8_t totalAddresses = sizeof(i2cAddressArray) / sizeof(uint8_t);

    if (i2cAddressIndex < totalAddresses)
    {
        _u8x8.i2c_address = i2cAddressArray[i2cAddressIndex];
    }
    else
    {
        // invalid address index, use default (don't set custom address)
    }

    if (success)
    {
        if (!_hwa.init())
            return false;

        /* setup display info */
        u8x8_SetupMemory(&_u8x8);
        u8x8_InitDisplay(&_u8x8);

        clearDisplay();
        setPowerSave(0);

        return true;
    }

    return false;
}

bool U8X8::deInit()
{
    if (_hwa.deInit())
    {
        u8x8_SetupDefaults(&_u8x8);
        _rows    = 0;
        _columns = 0;
        return true;
    }

    return false;
}

uint8_t U8X8::getColumns()
{
    return _columns;
}

uint8_t U8X8::getRows()
{
    return _rows;
}

void U8X8::clearDisplay()
{
    u8x8_ClearDisplay(&_u8x8);
}

void U8X8::setPowerSave(uint8_t is_enable)
{
    u8x8_SetPowerSave(&_u8x8, is_enable);
}

void U8X8::setFlipMode(uint8_t mode)
{
    u8x8_SetFlipMode(&_u8x8, mode);
}

void U8X8::setFont(const uint8_t* font_8x8)
{
    u8x8_SetFont(&_u8x8, font_8x8);
}

void U8X8::drawGlyph(uint8_t x, uint8_t y, uint8_t encoding)
{
    u8x8_DrawGlyph(&_u8x8, x, y, encoding);
}