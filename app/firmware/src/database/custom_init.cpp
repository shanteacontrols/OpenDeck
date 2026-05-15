/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/database/database.h"
#include "firmware/src/io/digital/switches/common.h"
#include "firmware/src/io/analog/common.h"
#include "firmware/src/io/outputs/common.h"
#include "firmware/src/protocol/midi/common.h"
#include "firmware/src/protocol/osc/common.h"

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

void Admin::custom_init_switches()
{
    for (size_t group = 0; group < switches::Collection::groups(); group++)
    {
        for (size_t i = 0; i < switches::Collection::size(group); i++)
        {
            update(Config::Section::Switch::MidiId, i + switches::Collection::start_index(group), i);
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

void Admin::custom_init_outputs()
{
    for (size_t group = 0; group < outputs::Collection::groups(); group++)
    {
        for (size_t i = 0; i < outputs::Collection::size(group); i++)
        {
            update(Config::Section::Outputs::ActivationId, i + outputs::Collection::start_index(group), i);
            update(Config::Section::Outputs::ControlType, i + outputs::Collection::start_index(group), outputs::ControlType::MidiInNoteMultiVal);
        }
    }
}

void Admin::custom_init_display()
{
}

void Admin::custom_init_touchscreen()
{
}
