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

#include "zlibs/drivers/uart/uart_hw.h"
#include "zlibs/utils/misc/ring_buffer.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>

#define OPENDECK_TOUCHSCREEN_NODE DT_CHOSEN(opendeck_touchscreen)

#if !DT_NODE_HAS_PROP(OPENDECK_TOUCHSCREEN_NODE, uart)
#error "Chosen OpenDeck touchscreen node must define a UART phandle."
#endif
#define OPENDECK_TOUCHSCREEN_UART_NODE DT_PHANDLE(OPENDECK_TOUCHSCREEN_NODE, uart)

namespace io::touchscreen
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        bool init() override
        {
            _initialized = _uart.init(_config);
            return _initialized;
        }

        bool deInit() override
        {
            const bool result = _uart.deinit();
            _initialized      = false;
            return result;
        }

        bool write(uint8_t value) override
        {
            return _uart.write(value);
        }

        std::optional<uint8_t> read() override
        {
            return _uart.read();
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            return interface == io::common::Allocatable::interface_t::UART ? _initialized : false;
        }

        private:
        static constexpr size_t                     BUFFER_SIZE = 256;
        static constexpr uint32_t                   BAUDRATE    = 38400;
        const device* const                         _device     = DEVICE_DT_GET(OPENDECK_TOUCHSCREEN_UART_NODE);
        zlibs::utils::misc::RingBuffer<BUFFER_SIZE> _rx_buffer  = {};
        zlibs::utils::misc::RingBuffer<BUFFER_SIZE> _tx_buffer  = {};
        zlibs::drivers::uart::Config                _config     = zlibs::drivers::uart::Config(
            BAUDRATE,
            UART_CFG_STOP_BITS_1,
            _rx_buffer,
            _tx_buffer);
        zlibs::drivers::uart::UartHw _uart        = zlibs::drivers::uart::UartHw(_device);
        bool                         _initialized = false;
    };
}    // namespace io::touchscreen
