/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "database/database.h"
#include "io/digital/buttons/common.h"
#include "io/analog/common.h"
#include "io/leds/common.h"
#include "protocol/midi/common.h"
#include "protocol/osc/common.h"

using namespace opendeck::database;
using namespace opendeck::io;
using namespace opendeck::protocol;

// each new group of components should have their IDs start from 0

void Admin::custom_init_global()
{
    // set global channel to 1
    update(Config::Section::Global::MidiSettings, midi::Setting::GlobalChannel, 1);

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC
    update(Config::Section::Global::OscSettings, osc::Setting::DestPort, osc::DEFAULT_DEST_PORT);
    update(Config::Section::Global::OscSettings, osc::Setting::ListenPort, osc::DEFAULT_LISTEN_PORT);
#endif
}

void Admin::custom_init_buttons()
{
    for (size_t group = 0; group < buttons::Collection::groups(); group++)
    {
        for (size_t i = 0; i < buttons::Collection::size(group); i++)
        {
            update(Config::Section::Button::MidiId, i + buttons::Collection::start_index(group), i);
        }
    }
}

void Admin::custom_init_encoders()
{
}

void Admin::custom_init_analog()
{
    for (size_t group = 0; group < analog::Collection::groups(); group++)
    {
        for (size_t i = 0; i < analog::Collection::size(group); i++)
        {
            update(Config::Section::Analog::MidiId, i + analog::Collection::start_index(group), i);
        }
    }
}

void Admin::custom_init_leds()
{
    for (size_t group = 0; group < leds::Collection::groups(); group++)
    {
        for (size_t i = 0; i < leds::Collection::size(group); i++)
        {
            update(Config::Section::Leds::ActivationId, i + leds::Collection::start_index(group), i);
            update(Config::Section::Leds::ControlType, i + leds::Collection::start_index(group), leds::ControlType::MidiInNoteMultiVal);
        }
    }
}

void Admin::custom_init_display()
{
}

void Admin::custom_init_touchscreen()
{
}
