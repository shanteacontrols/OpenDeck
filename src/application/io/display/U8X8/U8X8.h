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

#pragma once

#include "u8g2/csrc/u8x8.h"

namespace IO
{
    class U8X8
    {
        public:
        class HWAI2C
        {
            public:
            virtual bool init()                                             = 0;
            virtual bool deInit()                                           = 0;
            virtual bool write(uint8_t address, uint8_t* data, size_t size) = 0;
        };

        enum class displayController_t : uint8_t
        {
            invalid,
            ssd1306,
            AMOUNT
        };

        enum displayResolution_t : uint8_t
        {
            invalid,
            _128x64,
            _128x32,
            AMOUNT
        };

        const uint8_t i2cAddressArray[3] = {
            0x00,
            0x78,
            0x7A
        };

        U8X8(HWAI2C& hwa)
            : hwa(hwa)
        {}

        bool    init(uint8_t i2cAddressIndex, displayController_t controller, displayResolution_t resolution);
        bool    deInit();
        uint8_t getColumns();
        uint8_t getRows();
        void    clearDisplay();
        void    setPowerSave(uint8_t is_enable);
        void    setFlipMode(uint8_t mode);
        void    setFont(const uint8_t* font_8x8);
        void    drawGlyph(uint8_t x, uint8_t y, uint8_t encoding);

        private:
        HWAI2C& hwa;

        u8x8_t u8x8;
        size_t rows    = 0;
        size_t columns = 0;
    };
}    // namespace IO