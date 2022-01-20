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

void Database::customInitButtons()
{
    for (size_t group = 0; group < IO::Buttons::Collection::groups(); group++)
    {
        for (size_t i = 0; i < IO::Buttons::Collection::size(group); i++)
        {
            update(Database::Section::button_t::midiID, i + IO::Buttons::Collection::startIndex(group), i);
        }
    }
}

void Database::customInitAnalog()
{
    for (size_t group = 0; group < IO::Analog::Collection::groups(); group++)
    {
        for (size_t i = 0; i < IO::Analog::Collection::size(group); i++)
        {
            update(Database::Section::analog_t::midiID, i + IO::Analog::Collection::startIndex(group), i);
        }
    }
}

void Database::customInitLEDs()
{
    for (size_t group = 0; group < IO::LEDs::Collection::groups(); group++)
    {
        for (size_t i = 0; i < IO::LEDs::Collection::size(group); i++)
        {
            update(Database::Section::leds_t::activationID, i + IO::LEDs::Collection::startIndex(group), i);
            update(Database::Section::leds_t::controlType, i + IO::LEDs::Collection::startIndex(group), IO::LEDs::controlType_t::midiInNoteMultiVal);
        }
    }
}