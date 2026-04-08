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

namespace protocol::midi
{
    template<class Inherit, typename Data>
    class WriteParserHWA : public Inherit
    {
        public:
        WriteParserHWA() = default;

        bool init() override
        {
            return true;
        }

        bool deInit() override
        {
            return true;
        }

        bool write(Data& data) override
        {
            return false;
        }

        bool read(Data& data) override
        {
            if (_toDecode.size())
            {
                data = _toDecode.at(0);
                _toDecode.erase(_toDecode.begin());

                return true;
            }

            return false;
        }

        uint32_t time()
        {
            return 0;
        }

        bool supported()
        {
            return true;
        }

        bool setLoopback(bool state)
        {
            return true;
        }

        bool allocated(io::common::Allocatable::interface_t interface)
        {
            return false;
        }

        std::vector<Data>      _toDecode;
        std::vector<message_t> _decoded;
    };

    template<typename Transport, typename Hwa, typename Data>
    class WriteParser
    {
        public:
        WriteParser()
        {
            _base.init();
        }

        void feed(Data data)
        {
            _writeParserHWA._toDecode.push_back(data);

            while (_base.read())
            {
                _writeParserHWA._decoded.push_back(_base.message());
            }
        }

        std::vector<message_t>& writtenMessages()
        {
            return _writeParserHWA._decoded;
        }

        size_t totalWrittenChannelMessages()
        {
            size_t cnt = 0;

            for (size_t i = 0; i < _writeParserHWA._decoded.size(); i++)
            {
                if (IS_CHANNEL_MESSAGE(_writeParserHWA._decoded.at(i).type))
                {
                    cnt++;
                }
            }

            return cnt;
        }

        void clear()
        {
            _writeParserHWA._toDecode.clear();
            _writeParserHWA._decoded.clear();
        }

        private:
        WriteParserHWA<Hwa, Data> _writeParserHWA;
        Transport                 _base = Transport(_writeParserHWA);
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

        bool deInit() override
        {
            clear();
            return true;
        }

        bool read(UsbPacket& packet) override
        {
            if (!_readPackets.size())
            {
                return false;
            }

            packet = _readPackets.at(0);
            _readPackets.erase(_readPackets.begin());

            return true;
        }

        bool write(UsbPacket& packet) override
        {
            _writePackets.push_back(packet);
            _writeParser.feed(packet);

            return true;
        }

        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
            _writeParser.clear();
        }

        std::vector<UsbPacket>              _readPackets  = {};
        std::vector<UsbPacket>              _writePackets = {};
        WriteParser<Usb, HwaUsb, UsbPacket> _writeParser;
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
        MOCK_METHOD0(deInit, bool());
        MOCK_METHOD1(setLoopback, bool(bool state));

        bool read(SerialPacket& data) override
        {
            if (!_readPackets.size())
            {
                return false;
            }

            data = _readPackets.at(0);
            _readPackets.erase(_readPackets.begin());

            return true;
        }

        bool write(SerialPacket& data) override
        {
            _writePackets.push_back(data);
            _writeParser.feed(data);

            return true;
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            return false;
        }

        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
            _writeParser.clear();

            _loopbackEnabled = false;
        }

        std::vector<SerialPacket>                    _readPackets     = {};
        std::vector<SerialPacket>                    _writePackets    = {};
        bool                                         _loopbackEnabled = false;
        WriteParser<Serial, HwaSerial, SerialPacket> _writeParser;
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
        MOCK_METHOD0(deInit, bool());

        bool write(BlePacket& data) override
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

        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
        }

        std::vector<uint8_t>                _readPackets  = {};
        std::vector<uint8_t>                _writePackets = {};
        WriteParser<Ble, HwaBle, BlePacket> _writeParser;
    };
}    // namespace protocol::midi
