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

#include "database/Database.h"
#include "io/buttons/Buttons.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"

// each new group of components should have their IDs start from 0

void Database::Admin::customInitGlobal()
{
    // set global channel to 1
    update(Database::Config::Section::global_t::MIDI_SETTINGS, MIDI::setting_t::GLOBAL_CHANNEL, 1);
}

void Database::Admin::customInitButtons()
{
    for (size_t group = 0; group < IO::Buttons::Collection::groups(); group++)
    {
        for (size_t i = 0; i < IO::Buttons::Collection::size(group); i++)
        {
            update(Database::Config::Section::button_t::MIDI_ID, i + IO::Buttons::Collection::startIndex(group), i);
        }
    }
}

void Database::Admin::customInitAnalog()
{
    for (size_t group = 0; group < IO::Analog::Collection::groups(); group++)
    {
        for (size_t i = 0; i < IO::Analog::Collection::size(group); i++)
        {
            update(Database::Config::Section::analog_t::MIDI_ID, i + IO::Analog::Collection::startIndex(group), i);
        }
    }
}

void Database::Admin::customInitLEDs()
{
    for (size_t group = 0; group < IO::LEDs::Collection::groups(); group++)
    {
        for (size_t i = 0; i < IO::LEDs::Collection::size(group); i++)
        {
            update(Database::Config::Section::leds_t::ACTIVATION_ID, i + IO::LEDs::Collection::startIndex(group), i);
            update(Database::Config::Section::leds_t::CONTROL_TYPE, i + IO::LEDs::Collection::startIndex(group), IO::LEDs::controlType_t::MIDI_IN_NOTE_MULTI_VAL);
        }
    }
}