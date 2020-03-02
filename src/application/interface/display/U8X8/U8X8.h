/*

Copyright 2015-2020 Igor Petrovic

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

#include "u8g2/csrc/u8x8.h"

namespace U8X8
{
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

    bool    initDisplay(displayController_t controller, displayResolution_t resolution);
    uint8_t getColumns();
    uint8_t getRows();
    void    clearDisplay();
    void    setPowerSave(uint8_t is_enable);
    void    setFlipMode(uint8_t mode);
    void    setFont(const uint8_t* font_8x8);
    void    drawGlyph(uint8_t x, uint8_t y, uint8_t encoding);
}    // namespace U8X8