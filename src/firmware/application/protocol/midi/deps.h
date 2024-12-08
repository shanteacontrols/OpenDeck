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
#include "application/io/common/common.h"
#include "application/database/database.h"

namespace protocol::midi
{
    using Database = database::User<database::Config::Section::global_t>;

    class HwaUsb : public lib::midi::usb::Hwa
    {
        public:
        virtual ~HwaUsb() = default;

        virtual bool supported() = 0;
    };

    class HwaSerial : public io::common::Allocatable, public lib::midi::serial::Hwa
    {
        public:
        virtual ~HwaSerial() = default;

        virtual bool supported()             = 0;
        virtual bool setLoopback(bool state) = 0;
    };

    class HwaBle : public lib::midi::ble::Hwa
    {
        public:
        virtual ~HwaBle() = default;

        virtual bool supported() = 0;
    };
}    // namespace protocol::midi