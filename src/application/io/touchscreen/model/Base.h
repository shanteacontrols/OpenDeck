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
#include <stddef.h>

namespace IO
{
    class TouchscreenBase
    {
        public:
        class Common
        {
            public:
            Common() = default;

            protected:
            static constexpr size_t bufferSize = 50;
            static uint8_t          rxBuffer[bufferSize];
            static size_t           bufferCount;
        };

        enum class model_t : uint8_t
        {
            nextion,
            viewtech
        };

        struct icon_t
        {
            uint16_t xPos      = 0;
            uint16_t yPos      = 0;
            uint16_t width     = 0;
            uint16_t height    = 0;
            uint16_t onScreen  = 0;
            uint16_t offScreen = 0;
        };

        enum class tsEvent_t : uint8_t
        {
            none,
            button,
            coordinate
        };

        enum class pressType_t : uint8_t
        {
            none,
            initial,
            hold
        };

        struct tsData_t
        {
            pressType_t pressType   = pressType_t::none;
            size_t      buttonID    = 0;
            bool        buttonState = false;
            uint16_t    xPos        = 0;
            uint16_t    yPos        = 0;
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

        class HWA
        {
            public:
            virtual bool init()               = 0;
            virtual bool deInit()             = 0;
            virtual bool write(uint8_t value) = 0;
            virtual bool read(uint8_t& value) = 0;
        };

        virtual bool      init()                                 = 0;
        virtual bool      deInit()                               = 0;
        virtual bool      setScreen(size_t screenID)             = 0;
        virtual tsEvent_t update(tsData_t& tsData)               = 0;
        virtual void      setIconState(icon_t& icon, bool state) = 0;
        virtual bool      setBrightness(brightness_t brightness) = 0;
    };
}    // namespace IO