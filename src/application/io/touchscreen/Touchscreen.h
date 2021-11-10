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
#include "model/nextion/Nextion.h"
#include "model/viewtech/Viewtech.h"
#include "io/common/Common.h"

#ifndef TOUCHSCREEN_SUPPORTED
#include "stub/Touchscreen.h"
#else

namespace IO
{
    class Touchscreen
    {
        public:
        class Collection : public Common::BaseCollection<NR_OF_TOUCHSCREEN_COMPONENTS>
        {
            public:
            Collection() = delete;
        };

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

        using model_t      = TouchscreenBase::model_t;
        using icon_t       = TouchscreenBase::icon_t;
        using tsEvent_t    = TouchscreenBase::tsEvent_t;
        using pressType_t  = TouchscreenBase::pressType_t;
        using tsData_t     = TouchscreenBase::tsData_t;
        using brightness_t = TouchscreenBase::brightness_t;

        Touchscreen(TouchscreenBase::HWA& hwa,
                    Database&             database,
                    CDCPassthrough&       cdcPassthrough);

        bool   init(mode_t mode);
        bool   deInit(mode_t mode);
        bool   isInitialized() const;
        bool   isInitialized(mode_t mode) const;
        void   update();
        void   setScreen(size_t screenID);
        size_t activeScreen();
        void   registerEventNotifier(EventNotifier& eventNotifer);
        void   setIconState(size_t index, bool state);
        bool   setBrightness(brightness_t brightness);

        private:
        enum class analogType_t : uint8_t
        {
            horizontal,
            vertical
        };

        IO::TouchscreenBase& modelInstance();
        void                 processButton(const size_t buttonID, const bool state);
        void                 processCoordinate(pressType_t pressType, uint16_t xPos, uint16_t yPos);

        TouchscreenBase::HWA&        _hwa;
        Database&                    _database;
        CDCPassthrough&              _cdcPassthrough;
        EventNotifier*               _eventNotifier                                 = nullptr;
        size_t                       _activeScreenID                                = 0;
        bool                         _initialized                                   = false;
        mode_t                       _mode                                          = mode_t::normal;
        Nextion                      _nextion                                       = Nextion(_hwa);
        Viewtech                     _viewtech                                      = Viewtech(_hwa);
        IO::TouchscreenBase::model_t _activeModel                                   = TouchscreenBase::model_t::nextion;
        bool                         _analogActive[Collection::size()]              = {};
        uint8_t                      _txBuffer[TSCREEN_CDC_PASSTHROUGH_BUFFER_SIZE] = {};
        uint8_t                      _rxBuffer[TSCREEN_CDC_PASSTHROUGH_BUFFER_SIZE] = {};
    };
}    // namespace IO

#endif