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

#include "System.h"

Database::block_t System::dbBlock(uint8_t index)
{
    // sysex blocks and db blocks don't have 1/1 mapping

    auto sysExBlock = static_cast<block_t>(index);

    switch (sysExBlock)
    {
    case block_t::global:
        return Database::block_t::global;

    case block_t::buttons:
        return Database::block_t::buttons;

    case block_t::encoders:
        return Database::block_t::encoders;

    case block_t::analog:
        return Database::block_t::analog;

    case block_t::leds:
        return Database::block_t::leds;

    case block_t::display:
        return Database::block_t::display;

    case block_t::touchscreen:
        return Database::block_t::touchscreen;

    default:
        return Database::block_t::AMOUNT;
    }
}

Database::Section::global_t System::dbSection(Section::global_t section)
{
    return _sysEx2DB_global[static_cast<uint8_t>(section)];
}

Database::Section::button_t System::dbSection(Section::button_t section)
{
    return _sysEx2DB_button[static_cast<uint8_t>(section)];
}

Database::Section::encoder_t System::dbSection(Section::encoder_t section)
{
    return _sysEx2DB_encoder[static_cast<uint8_t>(section)];
}

Database::Section::analog_t System::dbSection(Section::analog_t section)
{
    return _sysEx2DB_analog[static_cast<uint8_t>(section)];
}

Database::Section::leds_t System::dbSection(Section::leds_t section)
{
    return _sysEx2DB_leds[static_cast<uint8_t>(section)];
}

Database::Section::display_t System::dbSection(Section::display_t section)
{
    return _sysEx2DB_display[static_cast<uint8_t>(section)];
}

Database::Section::touchscreen_t System::dbSection(Section::touchscreen_t section)
{
    return _sysEx2DB_touchscreen[static_cast<uint8_t>(section)];
}

bool System::isMIDIfeatureEnabled(midiFeature_t feature)
{
    return _database.read(Database::Section::global_t::midiFeatures, feature);
}

System::midiMergeType_t System::midiMergeType()
{
    return static_cast<midiMergeType_t>(_database.read(Database::Section::global_t::midiMerge, midiMerge_t::mergeType));
}