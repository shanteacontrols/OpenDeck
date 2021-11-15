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

#include "Conversion.h"

namespace
{
    const Database::Section::global_t _sysEx2DB_global[static_cast<uint8_t>(System::Config::Section::global_t::AMOUNT)] = {
        Database::Section::global_t::midiFeatures,
        Database::Section::global_t::midiMerge,
        Database::Section::global_t::AMOUNT,    // unused
        Database::Section::global_t::dmx,
    };

    const Database::Section::button_t _sysEx2DB_button[static_cast<uint8_t>(System::Config::Section::button_t::AMOUNT)] = {
        Database::Section::button_t::type,
        Database::Section::button_t::midiMessage,
        Database::Section::button_t::midiID,
        Database::Section::button_t::velocity,
        Database::Section::button_t::midiChannel
    };

    const Database::Section::encoder_t _sysEx2DB_encoder[static_cast<uint8_t>(System::Config::Section::encoder_t::AMOUNT)] = {
        Database::Section::encoder_t::enable,
        Database::Section::encoder_t::invert,
        Database::Section::encoder_t::mode,
        Database::Section::encoder_t::midiID,
        Database::Section::encoder_t::midiChannel,
        Database::Section::encoder_t::pulsesPerStep,
        Database::Section::encoder_t::acceleration,
        Database::Section::encoder_t::midiID,
        Database::Section::encoder_t::remoteSync
    };

    const Database::Section::analog_t _sysEx2DB_analog[static_cast<uint8_t>(System::Config::Section::analog_t::AMOUNT)] = {
        Database::Section::analog_t::enable,
        Database::Section::analog_t::invert,
        Database::Section::analog_t::type,
        Database::Section::analog_t::midiID,
        Database::Section::analog_t::midiID,
        Database::Section::analog_t::lowerLimit,
        Database::Section::analog_t::lowerLimit,
        Database::Section::analog_t::upperLimit,
        Database::Section::analog_t::upperLimit,
        Database::Section::analog_t::midiChannel
    };

    const Database::Section::leds_t _sysEx2DB_leds[static_cast<uint8_t>(System::Config::Section::leds_t::AMOUNT)] = {
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::global,
        Database::Section::leds_t::activationID,
        Database::Section::leds_t::rgbEnable,
        Database::Section::leds_t::controlType,
        Database::Section::leds_t::activationValue,
        Database::Section::leds_t::midiChannel,
    };

    const Database::Section::display_t _sysEx2DB_display[static_cast<uint8_t>(System::Config::Section::display_t::AMOUNT)] = {
        Database::Section::display_t::features,
        Database::Section::display_t::setting,
    };

    const Database::Section::touchscreen_t _sysEx2DB_touchscreen[static_cast<uint8_t>(System::Config::Section::touchscreen_t::AMOUNT)] = {
        Database::Section::touchscreen_t::setting,
        Database::Section::touchscreen_t::xPos,
        Database::Section::touchscreen_t::yPos,
        Database::Section::touchscreen_t::width,
        Database::Section::touchscreen_t::height,
        Database::Section::touchscreen_t::onScreen,
        Database::Section::touchscreen_t::offScreen,
        Database::Section::touchscreen_t::pageSwitchEnabled,
        Database::Section::touchscreen_t::pageSwitchIndex,
        Database::Section::touchscreen_t::analogPage,
        Database::Section::touchscreen_t::analogStartXCoordinate,
        Database::Section::touchscreen_t::analogEndXCoordinate,
        Database::Section::touchscreen_t::analogStartYCoordinate,
        Database::Section::touchscreen_t::analogEndYCoordinate,
        Database::Section::touchscreen_t::analogType,
        Database::Section::touchscreen_t::analogResetOnRelease,
    };
}    // namespace

namespace Util
{
    namespace Conversion
    {
        Database::Section::global_t sys2DBsection(System::Config::Section::global_t section)
        {
            return _sysEx2DB_global[static_cast<uint8_t>(section)];
        }

        Database::Section::button_t sys2DBsection(System::Config::Section::button_t section)
        {
            return _sysEx2DB_button[static_cast<uint8_t>(section)];
        }

        Database::Section::encoder_t sys2DBsection(System::Config::Section::encoder_t section)
        {
            return _sysEx2DB_encoder[static_cast<uint8_t>(section)];
        }

        Database::Section::analog_t sys2DBsection(System::Config::Section::analog_t section)
        {
            return _sysEx2DB_analog[static_cast<uint8_t>(section)];
        }

        Database::Section::leds_t sys2DBsection(System::Config::Section::leds_t section)
        {
            return _sysEx2DB_leds[static_cast<uint8_t>(section)];
        }

        Database::Section::display_t sys2DBsection(System::Config::Section::display_t section)
        {
            return _sysEx2DB_display[static_cast<uint8_t>(section)];
        }

        Database::Section::touchscreen_t sys2DBsection(System::Config::Section::touchscreen_t section)
        {
            return _sysEx2DB_touchscreen[static_cast<uint8_t>(section)];
        }
    }    // namespace Conversion
}    // namespace Util