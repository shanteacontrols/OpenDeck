/*

Copyright 2015-2022 Igor Petrovic

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

#ifdef USB_SUPPORTED
#ifdef USE_TINYUSB
#ifdef FW_APP

#include "board/Board.h"
#include "tusb.h"

namespace
{
    volatile uint32_t _baudRate;
}    // namespace

extern "C" void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* pLineCoding)
{
    _baudRate = pLineCoding->bit_rate;
    Board::USB::onCDCsetLineEncoding(_baudRate);
}

namespace Board::USB
{
    bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
    {
        tud_task();

        if (!tud_cdc_available())
        {
            return false;
        }

        size = tud_cdc_read(buffer, maxSize);

        return true;
    }

    bool readCDC(uint8_t& value)
    {
        size_t size;

        return readCDC(&value, size, 1);
    }

    bool writeCDC(uint8_t* buffer, size_t size)
    {
        tud_cdc_write(buffer, size);
        tud_cdc_write_flush();
        tud_task();

        return true;
    }

    bool writeCDC(uint8_t value)
    {
        return writeCDC(&value, 1);
    }
}    // namespace Board::USB

#endif
#endif
#endif