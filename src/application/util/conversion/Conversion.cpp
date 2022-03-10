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

#include "Conversion.h"

namespace
{
    const Database::Config::Section::global_t _sysEx2DB_global[static_cast<uint8_t>(System::Config::Section::global_t::AMOUNT)] = {
        Database::Config::Section::global_t::midiSettings,
        Database::Config::Section::global_t::AMOUNT,    // blank/reserved
        Database::Config::Section::global_t::AMOUNT,    // unused
        Database::Config::Section::global_t::dmx,
    };

    const Database::Config::Section::button_t _sysEx2DB_button[static_cast<uint8_t>(System::Config::Section::button_t::AMOUNT)] = {
        Database::Config::Section::button_t::type,
        Database::Config::Section::button_t::midiMessage,
        Database::Config::Section::button_t::midiID,
        Database::Config::Section::button_t::velocity,
        Database::Config::Section::button_t::midiChannel
    };

    const Database::Config::Section::encoder_t _sysEx2DB_encoder[static_cast<uint8_t>(System::Config::Section::encoder_t::AMOUNT)] = {
        Database::Config::Section::encoder_t::enable,
        Database::Config::Section::encoder_t::invert,
        Database::Config::Section::encoder_t::mode,
        Database::Config::Section::encoder_t::midiID,
        Database::Config::Section::encoder_t::midiChannel,
        Database::Config::Section::encoder_t::pulsesPerStep,
        Database::Config::Section::encoder_t::acceleration,
        Database::Config::Section::encoder_t::midiID,
        Database::Config::Section::encoder_t::remoteSync
    };

    const Database::Config::Section::analog_t _sysEx2DB_analog[static_cast<uint8_t>(System::Config::Section::analog_t::AMOUNT)] = {
        Database::Config::Section::analog_t::enable,
        Database::Config::Section::analog_t::invert,
        Database::Config::Section::analog_t::type,
        Database::Config::Section::analog_t::midiID,
        Database::Config::Section::analog_t::midiID,
        Database::Config::Section::analog_t::lowerLimit,
        Database::Config::Section::analog_t::lowerLimit,
        Database::Config::Section::analog_t::upperLimit,
        Database::Config::Section::analog_t::upperLimit,
        Database::Config::Section::analog_t::midiChannel,
        Database::Config::Section::analog_t::lowerOffset,
        Database::Config::Section::analog_t::upperOffset,
    };

    const Database::Config::Section::leds_t _sysEx2DB_leds[static_cast<uint8_t>(System::Config::Section::leds_t::AMOUNT)] = {
        Database::Config::Section::leds_t::AMOUNT,
        Database::Config::Section::leds_t::AMOUNT,
        Database::Config::Section::leds_t::global,
        Database::Config::Section::leds_t::activationID,
        Database::Config::Section::leds_t::rgbEnable,
        Database::Config::Section::leds_t::controlType,
        Database::Config::Section::leds_t::activationValue,
        Database::Config::Section::leds_t::midiChannel,
    };

    const Database::Config::Section::i2c_t _sysEx2DB_i2c[static_cast<uint8_t>(System::Config::Section::i2c_t::AMOUNT)] = {
        Database::Config::Section::i2c_t::display,
    };

    const Database::Config::Section::touchscreen_t _sysEx2DB_touchscreen[static_cast<uint8_t>(System::Config::Section::touchscreen_t::AMOUNT)] = {
        Database::Config::Section::touchscreen_t::setting,
        Database::Config::Section::touchscreen_t::xPos,
        Database::Config::Section::touchscreen_t::yPos,
        Database::Config::Section::touchscreen_t::width,
        Database::Config::Section::touchscreen_t::height,
        Database::Config::Section::touchscreen_t::onScreen,
        Database::Config::Section::touchscreen_t::offScreen,
        Database::Config::Section::touchscreen_t::pageSwitchEnabled,
        Database::Config::Section::touchscreen_t::pageSwitchIndex,
        Database::Config::Section::touchscreen_t::analogPage,
        Database::Config::Section::touchscreen_t::analogStartXCoordinate,
        Database::Config::Section::touchscreen_t::analogEndXCoordinate,
        Database::Config::Section::touchscreen_t::analogStartYCoordinate,
        Database::Config::Section::touchscreen_t::analogEndYCoordinate,
        Database::Config::Section::touchscreen_t::analogType,
        Database::Config::Section::touchscreen_t::analogResetOnRelease,
    };
}    // namespace

namespace Util::Conversion
{
    Database::Config::Section::global_t sys2DBsection(System::Config::Section::global_t section)
    {
        return _sysEx2DB_global[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::button_t sys2DBsection(System::Config::Section::button_t section)
    {
        return _sysEx2DB_button[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::encoder_t sys2DBsection(System::Config::Section::encoder_t section)
    {
        return _sysEx2DB_encoder[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::analog_t sys2DBsection(System::Config::Section::analog_t section)
    {
        return _sysEx2DB_analog[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::leds_t sys2DBsection(System::Config::Section::leds_t section)
    {
        return _sysEx2DB_leds[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::i2c_t sys2DBsection(System::Config::Section::i2c_t section)
    {
        return _sysEx2DB_i2c[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::touchscreen_t sys2DBsection(System::Config::Section::touchscreen_t section)
    {
        return _sysEx2DB_touchscreen[static_cast<uint8_t>(section)];
    }
}    // namespace Util::Conversion