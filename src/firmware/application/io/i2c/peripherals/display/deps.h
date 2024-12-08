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

#include "common.h"
#include "application/database/database.h"
#include "application/io/i2c/deps.h"

namespace io::i2c::display
{
    using Database = database::User<database::Config::Section::i2c_t>;
    using Hwa      = io::i2c::Hwa;
}    // namespace io::i2c::display