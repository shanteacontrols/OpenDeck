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

#include "deps.h"
#include "application/io/buttons/common.h"
#include "application/io/encoders/common.h"
#include "application/io/analog/common.h"
#include "application/io/leds/common.h"
#include "application/io/i2c/peripherals/display/common.h"
#include "application/io/touchscreen/common.h"
#include "application/protocol/midi/common.h"

namespace database
{
    class AppLayout : public database::Layout
    {
        public:
        AppLayout() = default;

        std::vector<lib::lessdb::Block>& layout(type_t type) override
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
        std::vector<lib::lessdb::Section> _systemSections = {
            // system section
            {
                static_cast<uint8_t>(database::Config::systemSetting_t::AMOUNT),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<lib::lessdb::Section> _globalSections = {
            // midi settings section
            {
                static_cast<uint8_t>(protocol::midi::setting_t::AMOUNT),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<lib::lessdb::Section> _buttonSections = {
            // type section
            {
                io::buttons::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // message type section
            {
                io::buttons::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // midi id section
            {
                io::buttons::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // value section
            {
                io::buttons::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                127,
            },

            // channel section
            {
                io::buttons::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                1,
            },
        };

        std::vector<lib::lessdb::Section> _encoderSections = {
            // encoder enabled section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // encoder inverted section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // encoding mode section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // midi id 1 section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::ENABLE,
                0,
            },

            // channel section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                1,
            },

            // pulses per step section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                4,
            },

            // acceleration section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // remote sync section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // lower value limit section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // upper value limit section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                16383,
            },

            // repeated value section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                127,
            },

            // midi id 2 section
            {
                io::encoders::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::ENABLE,
                0,
            },
        };

        std::vector<lib::lessdb::Section> _analogSections = {
            // analog enabled section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // analog inverted section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // analog type section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // midi id section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::ENABLE,
                0,
            },

            // lower value limit section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // upper value limit section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                16383,
            },

            // channel section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                1,
            },

            // lower adc percentage offset section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // upper adc percentage offset section
            {
                io::analog::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<lib::lessdb::Section> _ledSections = {
            // global parameters section
            {
                static_cast<size_t>(io::leds::setting_t::AMOUNT),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // activation id section
            {
                io::leds::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // rgb enabled section
            {
                (io::leds::Collection::SIZE() / 3) + (io::touchscreen::Collection::SIZE() / 3),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // led control type section
            {
                io::leds::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // single led activation value section
            {
                io::leds::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                127,
            },

            // channel section
            {
                io::leds::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                1,
            },
        };

        std::vector<lib::lessdb::Section> _i2cSections = {
            // display section
            {
                static_cast<uint8_t>(io::i2c::display::setting_t::AMOUNT),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<lib::lessdb::Section> _touchscreenSections = {
            // setting section
            {
                static_cast<uint8_t>(io::touchscreen::setting_t::AMOUNT),
                lib::lessdb::sectionParameterType_t::BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // x position section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // y position section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // width section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // height section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::WORD,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // on screen section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // off screen section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // page switch enabled section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::BIT,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },

            // page switch index section
            {
                io::touchscreen::Collection::SIZE(),
                lib::lessdb::sectionParameterType_t::HALF_BYTE,
                lib::lessdb::preserveSetting_t::DISABLE,
                lib::lessdb::autoIncrementSetting_t::DISABLE,
                0,
            },
        };

        std::vector<lib::lessdb::Block> _systemLayout = {
            // system block
            {
                _systemSections,
            },
        };

        std::vector<lib::lessdb::Block> _userLayout = {
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