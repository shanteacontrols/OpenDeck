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

#include <gmock/gmock.h>

namespace protocol::midi
{
    class WrittenMessageLog
    {
        public:
        void push(const midi_ump& packet)
        {
            _decoded.push_back(decode_message(packet));
        }

        void push(uint8_t data)
        {
            if (const auto packet = _parser.parse(data, 0); packet.has_value())
            {
                push(packet.value());
            }
        }

        std::vector<message_t>& writtenMessages()
        {
            return _decoded;
        }

        size_t totalWrittenChannelMessages() const
        {
            size_t count = 0;

            for (const auto& message : _decoded)
            {
                if (IS_CHANNEL_MESSAGE(message.type))
                {
                    count++;
                }
            }

            return count;
        }

        void clear()
        {
            _decoded.clear();
            _parser.reset();
        }

        private:
        lib::midi::Midi1ByteToUmpParser _parser  = {};
        std::vector<message_t>          _decoded = {};
    };

    class HwaUsbTest : public HwaUsb
    {
        public:
        HwaUsbTest() = default;

        bool supported() override
        {
            return true;
        }

        bool init() override
        {
            clear();
            return true;
        }

        bool deinit() override
        {
            clear();
            return true;
        }

        bool write(const midi_ump& packet) override
        {
            _writePackets.push_back(packet);
            _writeParser.push(packet);
            return true;
        }

        std::optional<midi_ump> read() override
        {
            if (_readPackets.empty())
            {
                return {};
            }

            const auto packet = _readPackets.front();
            _readPackets.erase(_readPackets.begin());
            return packet;
        }

        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
            _writeParser.clear();
        }

        std::vector<midi_ump> _readPackets  = {};
        std::vector<midi_ump> _writePackets = {};
        WrittenMessageLog     _writeParser;
    };

    class HwaSerialTest : public HwaSerial
    {
        public:
        HwaSerialTest() = default;

        bool supported() override
        {
#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
            return true;
#else
            return false;
#endif
        }

        MOCK_METHOD0(init, bool());
        MOCK_METHOD0(deinit, bool());
        MOCK_METHOD1(setLoopback, bool(bool state));

        bool write(uint8_t data) override
        {
            _writePackets.push_back(data);
            _writeParser.push(data);
            return true;
        }

        std::optional<uint8_t> read() override
        {
            if (_readPackets.empty())
            {
                return {};
            }

            const auto data = _readPackets.front();
            _readPackets.erase(_readPackets.begin());
            return data;
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            return interface == io::common::Allocatable::interface_t::UART ? _loopbackEnabled : false;
        }

        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
            _writeParser.clear();
            _loopbackEnabled = false;
        }

        std::vector<uint8_t> _readPackets     = {};
        std::vector<uint8_t> _writePackets    = {};
        bool                 _loopbackEnabled = false;
        WrittenMessageLog    _writeParser;
    };

    class HwaBleTest : public HwaBle
    {
        public:
        HwaBleTest() = default;

        bool supported() override
        {
#ifdef PROJECT_TARGET_SUPPORT_BLE
            return true;
#else
            return false;
#endif
        }

        MOCK_METHOD0(init, bool());
        MOCK_METHOD0(deinit, bool());

        bool write(BlePacket& packet) override
        {
            _writePackets.push_back(packet);
            return true;
        }

        std::optional<BlePacket> read() override
        {
            if (_readPackets.empty())
            {
                return {};
            }

            const auto packet = _readPackets.front();
            _readPackets.erase(_readPackets.begin());
            return packet;
        }

        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
        }

        std::vector<BlePacket> _readPackets  = {};
        std::vector<BlePacket> _writePackets = {};
        WrittenMessageLog      _writeParser;
    };
}    // namespace protocol::midi
