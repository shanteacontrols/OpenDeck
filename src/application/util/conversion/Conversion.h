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

#pragma once

#include "database/Database.h"
#include "system/Config.h"
#include "sysex/src/SysExConf.h"

namespace Util
{
    namespace Conversion
    {
        // Various helper functions used to convert between system config sections to
        // database sections.

        Database::Section::global_t      sys2DBsection(System::Config::Section::global_t section);
        Database::Section::button_t      sys2DBsection(System::Config::Section::button_t section);
        Database::Section::encoder_t     sys2DBsection(System::Config::Section::encoder_t section);
        Database::Section::analog_t      sys2DBsection(System::Config::Section::analog_t section);
        Database::Section::leds_t        sys2DBsection(System::Config::Section::leds_t section);
        Database::Section::i2c_t         sys2DBsection(System::Config::Section::i2c_t section);
        Database::Section::touchscreen_t sys2DBsection(System::Config::Section::touchscreen_t section);

        // alias split/merge classes from sysexconf to be globally available through this namespace
        using Split14bit = SysExConf::Split14bit;
        using Merge14bit = SysExConf::Merge14bit;
    }    // namespace Conversion
}    // namespace Util