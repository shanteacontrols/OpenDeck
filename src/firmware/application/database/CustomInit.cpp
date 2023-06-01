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

#include "application/database/Database.h"
#include "application/io/buttons/Buttons.h"
#include "application/io/analog/Analog.h"
#include "application/io/leds/LEDs.h"

// each new group of components should have their IDs start from 0

void database::Admin::customInitGlobal()
{
    // set global channel to 1
    update(database::Config::Section::global_t::MIDI_SETTINGS, MIDI::setting_t::GLOBAL_CHANNEL, 1);
}

void database::Admin::customInitButtons()
{
    for (size_t group = 0; group < io::Buttons::Collection::GROUPS(); group++)
    {
        for (size_t i = 0; i < io::Buttons::Collection::SIZE(group); i++)
        {
            update(database::Config::Section::button_t::MIDI_ID, i + io::Buttons::Collection::START_INDEX(group), i);
        }
    }
}

void database::Admin::customInitAnalog()
{
    for (size_t group = 0; group < io::Analog::Collection::GROUPS(); group++)
    {
        for (size_t i = 0; i < io::Analog::Collection::SIZE(group); i++)
        {
            update(database::Config::Section::analog_t::MIDI_ID, i + io::Analog::Collection::START_INDEX(group), i);
        }
    }
}

void database::Admin::customInitLEDs()
{
    for (size_t group = 0; group < io::LEDs::Collection::GROUPS(); group++)
    {
        for (size_t i = 0; i < io::LEDs::Collection::SIZE(group); i++)
        {
            update(database::Config::Section::leds_t::ACTIVATION_ID, i + io::LEDs::Collection::START_INDEX(group), i);
            update(database::Config::Section::leds_t::CONTROL_TYPE, i + io::LEDs::Collection::START_INDEX(group), io::LEDs::controlType_t::MIDI_IN_NOTE_MULTI_VAL);
        }
    }
}