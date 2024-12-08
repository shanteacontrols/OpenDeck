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

#include <inttypes.h>
#include <stddef.h>

namespace io::touchscreen
{
    enum class setting_t : uint8_t
    {
        ENABLE,
        MODEL,
        BRIGHTNESS,
        INITIAL_SCREEN,
        RESERVED,
        AMOUNT
    };

    enum class tsEvent_t : uint8_t
    {
        NONE,
        BUTTON,
    };

    enum class pressType_t : uint8_t
    {
        NONE,
        INITIAL,
        HOLD
    };

    enum class brightness_t : uint8_t
    {
        B10,
        B25,
        B50,
        B75,
        B80,
        B90,
        B100
    };

    enum class model_t : uint8_t
    {
        NEXTION,
        VIEWTECH,
        AMOUNT
    };

    struct Icon
    {
        uint16_t xPos      = 0;
        uint16_t yPos      = 0;
        uint16_t width     = 0;
        uint16_t height    = 0;
        uint16_t onScreen  = 0;
        uint16_t offScreen = 0;
    };

    struct Data
    {
        pressType_t pressType   = pressType_t::NONE;
        size_t      buttonIndex = 0;
        bool        buttonState = false;
        uint16_t    xPos        = 0;
        uint16_t    yPos        = 0;
    };

    class Model
    {
        public:
        virtual ~Model() = default;

        virtual bool      init()                                 = 0;
        virtual bool      deInit()                               = 0;
        virtual bool      setScreen(size_t index)                = 0;
        virtual tsEvent_t update(Data& tsData)                   = 0;
        virtual void      setIconState(Icon& icon, bool state)   = 0;
        virtual bool      setBrightness(brightness_t brightness) = 0;

        protected:
        static constexpr size_t BUFFER_SIZE = 50;
        static uint8_t          _rxBuffer[BUFFER_SIZE];
        static size_t           _bufferCount;
    };
}    // namespace io::touchscreen