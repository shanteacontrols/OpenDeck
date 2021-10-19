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

#include "system/System.h"

bool System::HWAButtons::state(size_t index, uint8_t& numberOfReadings, uint32_t& states)
{
    // if encoder under this index is enabled, just return false state each time
    if (_system._database.read(Database::Section::encoder_t::enable, _system._hwa.io().buttons().buttonToEncoderIndex(index)))
        return false;

    return _system._hwa.io().buttons().state(index, numberOfReadings, states);
}