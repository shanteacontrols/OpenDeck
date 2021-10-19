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

#pragma once

#include "Database.h"
#include "io/leds/LEDs.h"
#include "io/display/Display.h"
#include "io/touchscreen/Touchscreen.h"
#include "system/System.h"

#define MAX_PRESETS 10

namespace SectionPrivate
{
    enum class system_t : uint8_t
    {
        uid,
        presets,
        AMOUNT
    };
}

namespace
{
    // not user accessible
    LESSDB::section_t systemSections[static_cast<uint8_t>(SectionPrivate::system_t::AMOUNT)] = {
        // uid section
        {
            .numberOfParameters     = 1,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // presets
        {
            .numberOfParameters     = static_cast<uint8_t>(System::presetSetting_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        }
    };

    LESSDB::section_t globalSections[static_cast<uint8_t>(Database::Section::global_t::AMOUNT)] = {
        // midi feature section
        {
            .numberOfParameters     = static_cast<uint8_t>(System::midiFeature_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi merge section
        {
            .numberOfParameters     = static_cast<uint8_t>(System::midiMerge_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // dmx section
        {
            .numberOfParameters     = static_cast<uint8_t>(System::dmxSetting_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        }
    };

    LESSDB::section_t buttonSections[static_cast<uint8_t>(Database::Section::button_t::AMOUNT)] = {
        // type section
        {
            .numberOfParameters     = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi message type section
        {
            .numberOfParameters     = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi id section
        {
            .numberOfParameters     = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi velocity section
        {
            .numberOfParameters     = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 127,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi channel section
        {
            .numberOfParameters     = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        }
    };

    LESSDB::section_t encoderSections[static_cast<uint8_t>(Database::Section::encoder_t::AMOUNT)] = {
        // encoder enabled section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // encoder inverted section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // encoding mode section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi id section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = MAX_NUMBER_OF_ANALOG,
            .autoIncrement          = true,
            .address                = 0,
        },

        // midi channel section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // pulses per step section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 4,
            .autoIncrement          = false,
            .address                = 0,
        },

        // acceleration section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // remote sync section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ENCODERS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        }
    };

    LESSDB::section_t analogSections[static_cast<uint8_t>(Database::Section::analog_t::AMOUNT)] = {
        // analog enabled section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog inverted section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog type section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi id section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = true,
            .address                = 0,
        },

        // lower cc limit
        {
            .numberOfParameters     = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // upper cc limit
        {
            .numberOfParameters     = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 16383,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi channel section
        {
            .numberOfParameters     = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        }
    };

    LESSDB::section_t ledSections[static_cast<uint8_t>(Database::Section::leds_t::AMOUNT)] = {
        // global parameters section
        {
            .numberOfParameters     = static_cast<size_t>(IO::LEDs::setting_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // activation id section
        {
            .numberOfParameters     = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // rgb enabled section
        {
            .numberOfParameters     = MAX_NUMBER_OF_RGB_LEDS + (MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS / 3),
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // led control type section
        {
            .numberOfParameters     = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // single velocity value section
        {
            .numberOfParameters     = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 127,
            .autoIncrement          = false,
            .address                = 0,
        },

        // midi channel section
        {
            .numberOfParameters     = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        }
    };

    LESSDB::section_t displaySections[static_cast<uint8_t>(Database::Section::display_t::AMOUNT)] = {
        // features section
        {
            .numberOfParameters     = static_cast<uint8_t>(IO::Display::feature_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // setting section
        {
            .numberOfParameters     = static_cast<uint8_t>(IO::Display::setting_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        }
    };

    LESSDB::section_t touchscreenSections[static_cast<uint8_t>(Database::Section::touchscreen_t::AMOUNT)] = {
        // setting section
        {
            .numberOfParameters     = static_cast<uint8_t>(IO::Touchscreen::setting_t::AMOUNT),
            .parameterType          = LESSDB::sectionParameterType_t::byte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // x position section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // y position section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // width section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // height section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // on screen section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // off screen section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // page switch enabled section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // page switch index section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog page section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::halfByte,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog start x coordinate section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog end x coordinate section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog start y coordinate section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog end y coordinate section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::word,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog type section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },

        // analog reset on release section
        {
            .numberOfParameters     = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .parameterType          = LESSDB::sectionParameterType_t::bit,
            .preserveOnPartialReset = false,
            .defaultValue           = 0,
            .autoIncrement          = false,
            .address                = 0,
        },
    };

    LESSDB::block_t dbLayout[static_cast<uint8_t>(Database::block_t::AMOUNT) + 1] = {
        // system block
        {
            .numberOfSections = static_cast<uint8_t>(SectionPrivate::system_t::AMOUNT),
            .section          = systemSections,
            .address          = 0,
        },

        // global block
        {
            .numberOfSections = static_cast<uint8_t>(Database::Section::global_t::AMOUNT),
            .section          = globalSections,
            .address          = 0,
        },

        // buttons block
        {
            .numberOfSections = static_cast<uint8_t>(Database::Section::button_t::AMOUNT),
            .section          = buttonSections,
            .address          = 0,
        },

        // encoder block
        {
            .numberOfSections = static_cast<uint8_t>(Database::Section::encoder_t::AMOUNT),
            .section          = encoderSections,
            .address          = 0,
        },

        // analog block
        {
            .numberOfSections = static_cast<uint8_t>(Database::Section::analog_t::AMOUNT),
            .section          = analogSections,
            .address          = 0,
        },

        // led block
        {
            .numberOfSections = static_cast<uint8_t>(Database::Section::leds_t::AMOUNT),
            .section          = ledSections,
            .address          = 0,
        },

        // display block
        {
            .numberOfSections = static_cast<uint8_t>(Database::Section::display_t::AMOUNT),
            .section          = displaySections,
            .address          = 0,
        },

        // touchscreen block
        {
            .numberOfSections = static_cast<uint8_t>(Database::Section::touchscreen_t::AMOUNT),
            .section          = touchscreenSections,
            .address          = 0,
        },
    };
}    // namespace