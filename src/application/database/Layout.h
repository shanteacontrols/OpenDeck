/*

Copyright 2015-2019 Igor Petrovic

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

#pragma once

#include "Database.h"
#include "board/Board.h"
#include "interface/digital/output/leds/LEDs.h"
#include "blocks/System.h"

namespace
{
    //not user accessible
    LESSDB::section_t systemSections[DB_SECTIONS_SYSTEM] = {
        //uid section
        {
            .numberOfParameters = 1,
            .parameterType = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //settings
        {
            .numberOfParameters = SYSTEM_OPTIONS,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 }
    };

    LESSDB::section_t globalSections[DB_SECTIONS_GLOBAL] = {
        //midi feature section
        {
            .numberOfParameters = MIDI_FEATURES,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //midi merge section
        {
            .numberOfParameters = MIDI_MERGE_OPTIONS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 }
    };

    LESSDB::section_t buttonSections[DB_SECTIONS_BUTTONS] = {
        //type section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //midi message type section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //midi id section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = true,
            .address = 0 },

        //midi velocity section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 127,
            .autoIncrement = false,
            .address = 0 },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 }
    };

    LESSDB::section_t encoderSections[DB_SECTIONS_ENCODERS] = {
        //encoder enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //encoder inverted section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //encoding mode section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //midi id section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = true,
            .address = 0 },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //pulses per step section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 4,
            .autoIncrement = false,
            .address = 0 },

        //acceleration section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //remote sync section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 }
    };

    LESSDB::section_t analogSections[DB_SECTIONS_ANALOG] = {
        //analog enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //analog inverted section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //analog type section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //midi id section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .parameterType = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = true,
            .address = 0 },

        //lower cc limit
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .parameterType = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //upper cc limit
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .parameterType = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = 0,
            .defaultValue = 16383,
            .autoIncrement = false,
            .address = 0 },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 }
    };

#ifdef LEDS_SUPPORTED
    LESSDB::section_t ledSections[DB_SECTIONS_LEDS] = {
        //global parameters section
        {
            .numberOfParameters = static_cast<size_t>(Interface::digital::output::LEDs::setting_t::AMOUNT),
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //activation id section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = true,
            .address = 0 },

        //rgb enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_RGB_LEDS,
            .parameterType = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //led control type section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //single velocity value section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 127,
            .autoIncrement = false,
            .address = 0 },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .parameterType = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 }
    };
#endif

#ifdef DISPLAY_SUPPORTED
    LESSDB::section_t displaySections[DB_SECTIONS_DISPLAY] = {
        //features section
        {
            .numberOfParameters = DISPLAY_FEATURES,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 },

        //hw section
        {
            .numberOfParameters = DISPLAY_HW_PARAMETERS,
            .parameterType = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = 0,
            .defaultValue = 0,
            .autoIncrement = false,
            .address = 0 }
    };
#endif

    LESSDB::block_t dbLayout[DB_BLOCKS + 1] = {
        //system block
        {
            .numberOfSections = DB_SECTIONS_SYSTEM,
            .section = systemSections,
            .address = 0 },

        //global block
        {
            .numberOfSections = DB_SECTIONS_GLOBAL,
            .section = globalSections,
            .address = 0 },

        //buttons block
        {
            .numberOfSections = DB_SECTIONS_BUTTONS,
            .section = buttonSections,
            .address = 0 },

        //encoder block
        {
            .numberOfSections = DB_SECTIONS_ENCODERS,
            .section = encoderSections,
            .address = 0,
        },

        //analog block
        {
            .numberOfSections = DB_SECTIONS_ANALOG,
            .section = analogSections,
            .address = 0 },

#ifdef LEDS_SUPPORTED
        //led block
        {
            .numberOfSections = DB_SECTIONS_LEDS,
            .section = ledSections,
            .address = 0 },
#endif

#ifdef DISPLAY_SUPPORTED
        //display block
        {
            .numberOfSections = DB_SECTIONS_DISPLAY,
            .section = displaySections,
            .address = 0 }
#endif
    };
}    // namespace