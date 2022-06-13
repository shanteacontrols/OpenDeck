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

#pragma once

namespace IO
{
    class Touchscreen : public IO::Base
    {
        public:
        class Collection : public Common::BaseCollection<0>
        {
            public:
            Collection() = delete;
        };

        enum class setting_t : uint8_t
        {
            ENABLE,
            MODEL,
            BRIGHTNESS,
            INITIAL_SCREEN,
            CDC_PASSTHROUGH,
            AMOUNT
        };

        enum class mode_t : uint8_t
        {
            NORMAL,
            CDC_PASSTHROUGH
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
            NONE,
            BUTTON,
            COORDINATE
        };

        enum class pressType_t : uint8_t
        {
            NONE,
            INITIAL,
            HOLD
        };

        struct tsData_t
        {
            pressType_t pressType   = pressType_t::NONE;
            size_t      buttonID    = 0;
            bool        buttonState = false;
            uint16_t    xPos        = 0;
            uint16_t    yPos        = 0;
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

        class HWA : public ::IO::Common::Allocatable
        {
            public:
            virtual bool init()               = 0;
            virtual bool deInit()             = 0;
            virtual bool write(uint8_t value) = 0;
            virtual bool read(uint8_t& value) = 0;
        };

        class CDCPassthrough : public Common::Allocatable
        {
            public:
            virtual bool init()                                                       = 0;
            virtual bool deInit()                                                     = 0;
            virtual bool uartRead(uint8_t& value)                                     = 0;
            virtual bool uartWrite(uint8_t value)                                     = 0;
            virtual bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) = 0;
            virtual bool cdcWrite(uint8_t* buffer, size_t size)                       = 0;
        };

        class Model
        {
            public:
            virtual bool      init()                                 = 0;
            virtual bool      deInit()                               = 0;
            virtual bool      setScreen(size_t screenID)             = 0;
            virtual tsEvent_t update(tsData_t& tsData)               = 0;
            virtual void      setIconState(icon_t& icon, bool state) = 0;
            virtual bool      setBrightness(brightness_t brightness) = 0;
        };

        using Database = Database::User<Database::Config::Section::touchscreen_t>;

        Touchscreen(HWA&            hwa,
                    Database&       database,
                    CDCPassthrough& cdcPassthrough)
        {}

        bool init() override
        {
            return false;
        }

        void updateSingle(size_t index, bool forceRefresh = false) override
        {
        }

        void updateAll(bool forceRefresh = false) override
        {
        }

        size_t maxComponentUpdateIndex() override
        {
            return 0;
        }
    };
}    // namespace IO