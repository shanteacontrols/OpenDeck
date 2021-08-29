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

#include <inttypes.h>
#include <stdlib.h>
#include "database/Database.h"
#include "io/buttons/Buttons.h"
#include "io/analog/Analog.h"

namespace IO
{
    class Touchscreen
    {
        public:
        using icon_t = uint8_t;

        class Model
        {
            public:
            virtual bool init()                                              = 0;
            virtual bool setScreen(size_t screenID)                          = 0;
            virtual bool update(size_t& index, bool& state)                  = 0;
            virtual void setIconState(Touchscreen::icon_t& icon, bool state) = 0;
        };

        class EventNotifier
        {
            public:
            virtual void button(size_t index, bool state)                                 = 0;
            virtual void analog(size_t index, uint16_t value, uint16_t min, uint16_t max) = 0;
            virtual void screenChange(size_t screenID)                                    = 0;
        };

        class CDCPassthrough
        {
            public:
            virtual bool init()                                                       = 0;
            virtual bool deInit()                                                     = 0;
            virtual bool uartRead(uint8_t& byte)                                      = 0;
            virtual bool uartWrite(uint8_t byte)                                      = 0;
            virtual bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) = 0;
            virtual bool cdcWrite(uint8_t* buffer, size_t size)                       = 0;
        };

        Touchscreen(Database&       database,
                    ComponentInfo&  cInfo,
                    CDCPassthrough& cdcPassthrough)
        {}

        enum class setting_t : uint8_t
        {
            enable,
            brightness,
            AMOUNT
        };

        enum class brightness_t : uint8_t
        {
            _10,
            _25,
            _50,
            _75,
            _80,
            _90,
            _100
        };

        enum class analogType_t : uint8_t
        {
            horizontal,
            vertical
        };

        enum class mode_t : uint8_t
        {
            normal,
            cdcPassthrough
        };

        bool init(mode_t mode)
        {
            return false;
        }

        void update()
        {
        }

        void setScreen(size_t screenID)
        {
        }

        size_t activeScreen()
        {
            return 0;
        }

        void registerEventNotifier(EventNotifier& eventNotifer)
        {
        }

        void setIconState(size_t index, bool state)
        {
        }

        bool setBrightness(brightness_t brightness)
        {
            return false;
        }
    };
}    // namespace IO