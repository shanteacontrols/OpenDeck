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
#include "hwa_test.h"
#include "application/database/builder_test.h"

namespace protocol::midi
{
    class Builder
    {
        public:
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwaUsb, _hwaSerial, _hwaBle, _database)
        {}

        HwaUsbTest    _hwaUsb;
        HwaSerialTest _hwaSerial;
        HwaBleTest    _hwaBle;
        Database      _database;
        Midi          _instance;
    };
}    // namespace protocol::midi