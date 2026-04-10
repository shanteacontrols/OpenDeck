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

#pragma once

#include "digital.h"
#include "frame_store.h"
#include "drivers/driver.h"
#include "application/io/digital/buttons/buttons.h"
#include "application/io/digital/buttons/filter_hw.h"
#include "application/io/digital/buttons/hwa_hw.h"
#include "application/io/digital/encoders/encoders.h"
#include "application/io/digital/encoders/filter_hw.h"
#include "application/io/digital/encoders/hwa_hw.h"
#include "application/database/builder.h"

namespace io::digital
{
    class Builder
    {
        public:
        explicit Builder(database::Admin& database)
            : _buttonsDatabase(database)
            , _encodersDatabase(database)
            , _frameStore(_driver)
            , _buttonsHwa(_frameStore)
            , _buttons(_buttonsHwa, _buttonsFilter, _buttonsDatabase)
            , _encodersHwa(_frameStore)
            , _encoders(_encodersHwa, _encodersFilter, _encodersDatabase)
            , _instance(_driver, _frameStore, _buttons, _encoders)
        {}

        Digital& instance()
        {
            return _instance;
        }

        private:
        io::buttons::Database  _buttonsDatabase;
        io::encoders::Database _encodersDatabase;
        drivers::Driver        _driver;
        FrameStore             _frameStore;
        io::buttons::HwaHw     _buttonsHwa;
        io::buttons::FilterHw  _buttonsFilter;
        io::buttons::Buttons   _buttons;
        io::encoders::HwaHw    _encodersHwa;
        io::encoders::FilterHw _encodersFilter;
        io::encoders::Encoders _encoders;
        Digital                _instance;
    };
}    // namespace io::digital
