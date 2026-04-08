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
#include "application/messaging/messaging.h"
#include "board/board.h"

#include "core/mcu.h"

namespace protocol::midi
{
    class HwaUsbHw : public HwaUsb
    {
        public:
        HwaUsbHw() = default;

        bool supported() override
        {
            return true;
        }

        bool init() override
        {
            // usb might be already initialized
            return board::usb::init() != board::initStatus_t::ERROR;
        }

        bool deInit() override
        {
            return true;    // never deinit usb interface, just pretend here
        }

        bool read(UsbPacket& packet) override
        {
            if (board::usb::readMidi(packet))
            {
                board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                       board::io::indicators::direction_t::INCOMING);

                return true;
            }

            return false;
        }

        bool write(UsbPacket& packet) override
        {
            if (board::usb::writeMidi(packet))
            {
                board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                       board::io::indicators::direction_t::OUTGOING);

                return true;
            }

            return false;
        }
    };

    class HwaSerialHw : public HwaSerial
    {
        public:
        HwaSerialHw() = default;

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
        bool supported() override
        {
            return true;
        }

        bool init() override
        {
            static constexpr uint32_t BAUDRATE = 31250;
            return board::uart::init(PROJECT_TARGET_UART_CHANNEL_DIN, BAUDRATE) == board::initStatus_t::OK;
        }

        bool deInit() override
        {
            board::uart::deInit(PROJECT_TARGET_UART_CHANNEL_DIN);
            return true;
        }

        bool setLoopback(bool state) override
        {
            board::uart::setLoopbackState(PROJECT_TARGET_UART_CHANNEL_DIN, state);
            return true;
        }

        bool read(SerialPacket& packet) override
        {
            if (board::uart::read(PROJECT_TARGET_UART_CHANNEL_DIN, packet.data))
            {
                board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                       board::io::indicators::direction_t::INCOMING);

                return true;
            }

            return false;
        }

        bool write(SerialPacket& packet) override
        {
            if (board::uart::write(PROJECT_TARGET_UART_CHANNEL_DIN, packet.data))
            {
                board::io::indicators::indicateTraffic(board::io::indicators::source_t::UART,
                                                       board::io::indicators::direction_t::OUTGOING);

                return true;
            }

            return false;
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            if (interface == io::common::Allocatable::interface_t::UART)
            {
                return board::uart::isInitialized(PROJECT_TARGET_UART_CHANNEL_DIN);
            }

            return false;
        }
#else
        bool supported() override
        {
            return false;
        }

        bool init() override
        {
            return false;
        }

        bool deInit() override
        {
            return false;
        }

        bool setLoopback(bool state) override
        {
            return false;
        }

        bool read(SerialPacket& packet) override
        {
            return false;
        }

        bool write(SerialPacket& packet) override
        {
            return false;
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            return false;
        }
#endif
    };

    class HwaBleHw : public HwaBle
    {
        public:
        HwaBleHw() = default;

#ifdef PROJECT_TARGET_SUPPORT_BLE
        bool supported() override
        {
            return true;
        }

        bool init() override
        {
            return board::ble::init();
        }

        bool deInit() override
        {
            return board::ble::deInit();
        }

        bool write(BlePacket& packet) override
        {
            board::io::indicators::indicateTraffic(board::io::indicators::source_t::BLE,
                                                   board::io::indicators::direction_t::OUTGOING);

            return board::ble::midi::write(&packet.data[0], packet.size);
        }

        bool read(BlePacket& packet) override
        {
            if (board::ble::midi::read(&packet.data[0], packet.size, packet.data.size()))
            {
                board::io::indicators::indicateTraffic(board::io::indicators::source_t::BLE,
                                                       board::io::indicators::direction_t::INCOMING);

                return true;
            }

            return false;
        }

        uint32_t time() override
        {
            return core::mcu::timing::ms();
        }
#else
        bool supported() override
        {
            return false;
        }

        bool init() override
        {
            return false;
        }

        bool deInit() override
        {
            return false;
        }

        bool write(BlePacket& packet) override
        {
            return false;
        }

        bool read(BlePacket& data) override
        {
            return false;
        }

        uint32_t time() override
        {
            return 0;
        }
#endif
    };
}    // namespace protocol::midi