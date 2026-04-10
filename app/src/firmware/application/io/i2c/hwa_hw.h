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

#include "deps.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

namespace io::i2c
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        bool init() override
        {
            return device_is_ready(_device);
        }

        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return i2c_write(_device, buffer, size, address) == 0;
        }

        bool deviceAvailable(uint8_t address) override
        {
            return i2c_write(_device, nullptr, 0, address) == 0;
        }

        private:
        static_assert(DT_NODE_EXISTS(DT_CHOSEN(opendeck_display)), "Chosen OpenDeck display node must exist.");
        static_assert(DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_display), i2c), "OpenDeck display node must define an i2c phandle.");

        const device* const _device = DEVICE_DT_GET(DT_PHANDLE(DT_CHOSEN(opendeck_display), i2c));
    };
}    // namespace io::i2c
