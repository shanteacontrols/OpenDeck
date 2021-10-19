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

#include "database/Database.h"
#include "io/leds/LEDs.h"

void Database::customInitButtons()
{
    // each new category of buttons should have their IDs start from 0
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        update(Database::Section::button_t::midiID, i, i);

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        update(Database::Section::button_t::midiID, i + MAX_NUMBER_OF_BUTTONS, i);

    for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
        update(Database::Section::button_t::midiID, i + MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG, i);
}

void Database::customInitAnalog()
{
    for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
        update(Database::Section::analog_t::midiID, i + MAX_NUMBER_OF_ANALOG, i);
}

void Database::customInitLEDs()
{
    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
    {
        update(Database::Section::leds_t::activationID, i, i);
        update(Database::Section::leds_t::controlType, i, IO::LEDs::controlType_t::midiInNoteMultiVal);
    }

    for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
    {
        update(Database::Section::leds_t::activationID, i + MAX_NUMBER_OF_LEDS, i);
        update(Database::Section::leds_t::controlType, i + MAX_NUMBER_OF_LEDS, IO::LEDs::controlType_t::midiInNoteMultiVal);
    }
}