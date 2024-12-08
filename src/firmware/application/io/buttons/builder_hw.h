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

#include "buttons.h"
#include "filter_hw.h"
#include "hwa_hw.h"
#include "application/database/builder_hw.h"

namespace io::buttons
{
    class BuilderHw
    {
        public:
        BuilderHw() = default;

        Buttons& instance()
        {
            return _instance;
        }

        private:
        HwaHw    _hwa;
        FilterHw _filter;
        Database _database = Database(database::BuilderHw::instance());
        Buttons  _instance = Buttons(_hwa, _filter, _database);
    };
}    // namespace io::buttons