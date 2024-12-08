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

#include "midi.h"
#include "hwa_hw.h"
#include "application/database/builder_hw.h"

namespace protocol::midi
{
    class BuilderHw
    {
        public:
        BuilderHw() = default;

        Midi& instance()
        {
            return _instance;
        }

        private:
        HwaUsbHw    _hwaUsb;
        HwaSerialHw _hwaSerial;
        HwaBleHw    _hwaBle;
        Database    _database = Database(database::BuilderHw::instance());
        Midi        _instance = Midi(_hwaUsb, _hwaSerial, _hwaBle, _database);
    };
}    // namespace protocol::midi