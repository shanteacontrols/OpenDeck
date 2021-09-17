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
#include "io/touchscreen/model/Base.h"
#include "io/common/CInfo.h"

namespace IO
{
    class Touchscreen
    {
        public:
        enum class setting_t : uint8_t
        {
            enable,
            model,
            brightness,
            initialScreen,
            cdcPassthrough,
            AMOUNT
        };

        enum class mode_t : uint8_t
        {
            normal,
            cdcPassthrough
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
            virtual bool uartRead(uint8_t& value)                                     = 0;
            virtual bool uartWrite(uint8_t value)                                     = 0;
            virtual bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) = 0;
            virtual bool cdcWrite(uint8_t* buffer, size_t size)                       = 0;
        };

        using brightness_t = TouchscreenBase::brightness_t;

        Touchscreen(TouchscreenBase::HWA& hwa,
                    Database&             database,
                    ComponentInfo&        cInfo,
                    CDCPassthrough&       cdcPassthrough)
        {}

        bool init(mode_t mode)
        {
            return false;
        }

        bool deInit(mode_t mode)
        {
            return false;
        }

        bool isInitialized() const
        {
            return false;
        }

        bool isInitialized(mode_t mode)
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

        bool setBrightness(TouchscreenBase::brightness_t brightness)
        {
            return false;
        }
    };
}    // namespace IO