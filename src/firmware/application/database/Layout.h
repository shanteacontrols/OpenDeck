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

#include "Database.h"
#include "application/io/buttons/Buttons.h"
#include "application/io/encoders/Encoders.h"
#include "application/io/analog/Analog.h"
#include "application/io/leds/LEDs.h"
#include "application/io/i2c/peripherals/display/Display.h"
#include "application/io/touchscreen/Touchscreen.h"
#include "application/protocol/midi/MIDI.h"

namespace database
{
    class AppLayout : public database::Admin::Layout
    {
        public:
        AppLayout() = default;

        std::vector<LESSDB::Block>& layout(type_t type) override
        {
            switch (type)
            {
            case type_t::SYSTEM:
                return _systemLayout;

            default:
                return _userLayout;
            }
        }

        private:
        // not user accessible
        std::vector<LESSDB::Section> _systemSections = {
            // uid section
            {
                1,
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // presets
            {
                static_cast<uint8_t>(database::Config::presetSetting_t::AMOUNT),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<LESSDB::Section> _globalSections = {
            // midi settings section
            {
                static_cast<uint8_t>(protocol::MIDI::setting_t::AMOUNT),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<LESSDB::Section> _buttonSections = {
            // type section
            {
                io::Buttons::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // message type section
            {
                io::Buttons::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // midi id section
            {
                io::Buttons::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // value section
            {
                io::Buttons::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                127,
            },

            // channel section
            {
                io::Buttons::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                1,
            },
        };

        std::vector<LESSDB::Section> _encoderSections = {
            // encoder enabled section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // encoder inverted section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // encoding mode section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // midi id section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::ENABLE,
                0,
            },

            // channel section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                1,
            },

            // pulses per step section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                4,
            },

            // acceleration section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // remote sync section
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // lower value limit
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // upper value limit
            {
                io::Encoders::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                16383,
            },
        };

        std::vector<LESSDB::Section> _analogSections = {
            // analog enabled section
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // analog inverted section
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // analog type section
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // midi id section
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::ENABLE,
                0,
            },

            // lower value limit
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // upper value limit
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                16383,
            },

            // channel section
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                1,
            },

            // lower adc percentage offset
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // upper adc percentage offset
            {
                io::Analog::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<LESSDB::Section> _ledSections = {
            // global parameters section
            {
                static_cast<size_t>(io::LEDs::setting_t::AMOUNT),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // activation id section
            {
                io::LEDs::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // rgb enabled section
            {
                (io::LEDs::Collection::SIZE() / 3) + (io::Touchscreen::Collection::SIZE() / 3),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // led control type section
            {
                io::LEDs::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // single led activation value section
            {
                io::LEDs::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                127,
            },

            // channel section
            {
                io::LEDs::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                1,
            },
        };

        std::vector<LESSDB::Section> _i2cSections = {
            // display section
            {
                static_cast<uint8_t>(io::Display::setting_t::AMOUNT),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<LESSDB::Section> _touchscreenSections = {
            // setting section
            {
                static_cast<uint8_t>(io::Touchscreen::setting_t::AMOUNT),
                LESSDB::sectionParameterType_t::BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // x position section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // y position section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // width section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // height section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::WORD,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // on screen section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // off screen section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // page switch enabled section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::BIT,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },

            // page switch index section
            {
                io::Touchscreen::Collection::SIZE(),
                LESSDB::sectionParameterType_t::HALF_BYTE,
                LESSDB::preserveSetting_t::DISABLE,
                LESSDB::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<LESSDB::Block> _systemLayout = {
            // system block
            {
                _systemSections,
            },
        };

        std::vector<LESSDB::Block> _userLayout = {
            // global block
            {
                _globalSections,
            },

            // buttons block
            {
                _buttonSections,
            },

            // encoder block
            {
                _encoderSections,
            },

            // analog block
            {
                _analogSections,
            },

            // led block
            {
                _ledSections,
            },

            // display block
            {
                _i2cSections,
            },

            // touchscreen block
            {
                _touchscreenSections,
            },
        };
    };
}    // namespace database