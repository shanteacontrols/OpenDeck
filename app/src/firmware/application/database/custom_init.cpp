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

#include "application/database/database.h"
#include "application/io/buttons/common.h"
#include "application/io/analog/common.h"
#include "application/io/leds/common.h"
#include "application/protocol/midi/common.h"

using namespace database;
using namespace io;
using namespace protocol;

// each new group of components should have their IDs start from 0

void Admin::customInitGlobal()
{
    // set global channel to 1
    update(Config::Section::global_t::MIDI_SETTINGS, midi::setting_t::GLOBAL_CHANNEL, 1);
}

void Admin::customInitButtons()
{
    for (size_t group = 0; group < buttons::Collection::GROUPS(); group++)
    {
        for (size_t i = 0; i < buttons::Collection::SIZE(group); i++)
        {
            update(Config::Section::button_t::MIDI_ID, i + buttons::Collection::START_INDEX(group), i);
        }
    }
}

void Admin::customInitAnalog()
{
    for (size_t group = 0; group < analog::Collection::GROUPS(); group++)
    {
        for (size_t i = 0; i < analog::Collection::SIZE(group); i++)
        {
            update(Config::Section::analog_t::MIDI_ID, i + analog::Collection::START_INDEX(group), i);
        }
    }
}

void Admin::customInitLEDs()
{
    for (size_t group = 0; group < leds::Collection::GROUPS(); group++)
    {
        for (size_t i = 0; i < leds::Collection::SIZE(group); i++)
        {
            update(Config::Section::leds_t::ACTIVATION_ID, i + leds::Collection::START_INDEX(group), i);
            update(Config::Section::leds_t::CONTROL_TYPE, i + leds::Collection::START_INDEX(group), leds::controlType_t::MIDI_IN_NOTE_MULTI_VAL);
        }
    }
}