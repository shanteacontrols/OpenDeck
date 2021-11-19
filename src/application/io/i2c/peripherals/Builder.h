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

#include "io/i2c/I2C.h"
#include "database/Database.h"

#ifdef I2C_SUPPORTED

#include "display/Display.h"

namespace IO
{
    class I2CPeripheralBuilder
    {
        public:
        I2CPeripheralBuilder(I2C::HWA& hwa, Database& database);

        private:
        Display _display;
    };
}    // namespace IO

#else
#include "stub/Builder.h"
#endif