/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_USB
#ifdef BOARD_USE_TINYUSB
#ifdef FW_APP

#include "board/board.h"

#include "tusb.h"

namespace
{
    volatile uint32_t baudRate;
}    // namespace

extern "C" void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* pLineCoding)
{
    baudRate = pLineCoding->bit_rate;
    board::usb::onCdcSetLineEncoding(baudRate);
}

namespace board::usb
{
    bool readCdc(uint8_t* buffer, size_t& size, const size_t maxSize)
    {
        tud_task();

        if (!tud_cdc_available())
        {
            return false;
        }

        size = tud_cdc_read(buffer, maxSize);

        return true;
    }

    bool readCdc(uint8_t& value)
    {
        size_t size;

        return readCdc(&value, size, 1);
    }

    bool writeCdc(uint8_t* buffer, size_t size)
    {
        tud_cdc_write(buffer, size);
        tud_cdc_write_flush();
        tud_task();

        return true;
    }

    bool writeCdc(uint8_t value)
    {
        return writeCdc(&value, 1);
    }
}    // namespace board::usb

#endif
#endif
#endif