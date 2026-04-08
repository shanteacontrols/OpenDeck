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

#include "display/display.h"
#include "application/database/database.h"

namespace io::i2c
{
    class BuilderPeripherals
    {
        public:
        BuilderPeripherals(Hwa& hwa, database::Admin& database)
            : _displayDatabase(database)
            , _display(hwa, _displayDatabase)
        {}

        private:
        display::Database _displayDatabase;
        display::Display  _display;
    };
}    // namespace io::i2c