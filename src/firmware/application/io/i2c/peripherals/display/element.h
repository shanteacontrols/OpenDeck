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

#pragma once

#include "core/mcu.h"
#include "core/util/util.h"

#include <cstdio>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>

namespace io::i2c::display
{
    class DisplayTextControl
    {
        public:
        virtual uint8_t     MAX_LENGTH()                   = 0;
        virtual uint8_t     ROW()                          = 0;
        virtual uint8_t     COLUMN()                       = 0;
        virtual bool        USE_RETENTION()                = 0;
        virtual const char* text()                         = 0;
        virtual void        setText(const char* text, ...) = 0;
        virtual uint32_t    change()                       = 0;
        virtual void        clearChange()                  = 0;
        virtual uint32_t    lastUpdateTime()               = 0;
    };

    template<uint8_t maxLength, uint8_t row, uint8_t column, bool useRetention>
    class DisplayElement : public DisplayTextControl
    {
        public:
        DisplayElement() = default;

        uint8_t MAX_LENGTH() override
        {
            return maxLength;
        }

        uint8_t ROW() override
        {
            return row;
        }

        uint8_t COLUMN() override
        {
            return column;
        }

        bool USE_RETENTION() override
        {
            return useRetention;
        }

        const char* text() override
        {
            return _text;
        }

        void setText(const char* text, ...) override
        {
            static constexpr uint8_t TEXT_BUFFER_SIZE = 32;
            static_assert((maxLength + 1) <= TEXT_BUFFER_SIZE, "Provided element size too large");
            static char tempBuff[TEXT_BUFFER_SIZE] = {};

            va_list args;
            va_start(args, text);
            int len = vsnprintf(tempBuff, sizeof(tempBuff), text, args);
            va_end(args);

            if (len >= static_cast<int>(sizeof(tempBuff) - 1))
            {
                // overflow
                return;
            }

            memset(&tempBuff[strlen(tempBuff)], ' ', sizeof(tempBuff) - len - 1);
            tempBuff[sizeof(tempBuff) - 1] = '\0';

            for (size_t i = 0; i < maxLength; i++)
            {
                if (tempBuff[i] != _text[i])
                {
                    core::util::BIT_SET(_textChange, i);
                }
            }

            strncpy(_text, tempBuff, maxLength);
            _lastUpdateTime = core::mcu::timing::ms();
        }

        uint32_t change() override
        {
            return _textChange;
        }

        void clearChange() override
        {
            _textChange = 0;
        }

        uint32_t lastUpdateTime() override
        {
            return _lastUpdateTime;
        }

        private:
        char     _text[maxLength + 1] = {};
        uint32_t _textChange          = 0;
        uint32_t _lastUpdateTime      = 0;
    };
}    // namespace io::i2c::display