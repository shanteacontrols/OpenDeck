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

#ifdef USE_UART

#include "USBOverSerial.h"
#include "board/Board.h"

namespace Board
{
    namespace USBOverSerial
    {
        class USBPacketUpdater
        {
            public:
            USBPacketUpdater(USBReadPacket& packet)
                : _packet(packet)
            {}

            enum class appendResult_t : uint8_t
            {
                ok,
                outOfBounds,
                done
            };

            enum class framing_t : uint8_t
            {
                boundary          = 0x7E,
                escape            = 0x7D,
                escapeValueSuffix = 0x00
            };

            appendResult_t append(uint8_t value)
            {
                auto onBoundaryFound = [&]() {
                    // actual boundary value, reset the packet
                    _packet.reset();

                    //...but also set boundary flag
                    _packet._boundaryFound = true;
                };

                if (!_packet._boundaryFound)
                {
                    if (value == static_cast<uint8_t>(framing_t::boundary))
                        _packet._boundaryFound = true;
                }
                else
                {
                    if (!_packet._escapeProcessing)
                    {
                        if (value == static_cast<uint8_t>(framing_t::escape))
                        {
                            _packet._escapeProcessing = true;
                        }
                        else if (value == static_cast<uint8_t>(framing_t::boundary))
                        {
                            onBoundaryFound();
                            return appendResult_t::ok;
                        }
                    }
                    else
                    {
                        _packet._escapeProcessing = false;

                        if (value == static_cast<uint8_t>(framing_t::boundary))
                        {
                            // this is actual data, do nothing
                        }
                        else if (value == static_cast<uint8_t>(framing_t::escapeValueSuffix))
                        {
                            value = static_cast<uint8_t>(framing_t::escape);
                        }
                        else
                        {
                            onBoundaryFound();
                            return appendResult_t::ok;
                        }
                    }

                    if (!_packet._escapeProcessing)
                    {
                        if (_packet._type == packetType_t::invalid)
                        {
                            if (checkType(value))
                            {
                                _packet._type = static_cast<packetType_t>(value);
                            }
                        }
                        else if (!_packet._sizeSet)
                        {
                            _packet._size    = value;
                            _packet._count   = 0;
                            _packet._sizeSet = true;
                        }
                        else
                        {
                            // if the packet is larger than expected, ignore the rest of the packet
                            if (_packet._count < _packet._maxSize)
                                _packet._buffer[_packet._count++] = value;
                        }
                    }

                    if (_packet._sizeSet && (_packet._count == _packet._size))
                    {
                        _packet._done = true;

                        return appendResult_t::done;
                    }
                }

                return appendResult_t::ok;
            }

            private:
            USBReadPacket& _packet;

            bool checkType(uint8_t value)
            {
                switch (value)
                {
                case static_cast<uint8_t>(packetType_t::midi):
                case static_cast<uint8_t>(packetType_t::internal):
                case static_cast<uint8_t>(packetType_t::cdc):
                    return true;

                default:
                    return false;
                }
            }
        };

        bool read(uint8_t channel, USBReadPacket& packet)
        {
            if (packet.done())
                return true;

            uint8_t value;

            while (Board::UART::read(channel, value))
            {
                USBPacketUpdater updater(packet);

                if (updater.append(value) == USBPacketUpdater::appendResult_t::done)
                {
                    return true;
                }
            }

            return false;
        }

        bool write(uint8_t channel, USBWritePacket& packet)
        {
            auto writeSingle = [](uint8_t channel, uint8_t value, bool initial = false) {
                if (!initial && (value == static_cast<uint8_t>(USBPacketUpdater::framing_t::boundary)))
                {
                    // send escape first
                    if (!Board::UART::write(channel, static_cast<uint8_t>(USBPacketUpdater::framing_t::escape)))
                        return false;
                }

                if (!Board::UART::write(channel, value))
                    return false;

                if (value == static_cast<uint8_t>(USBPacketUpdater::framing_t::escape))
                {
                    if (!Board::UART::write(channel, static_cast<uint8_t>(USBPacketUpdater::framing_t::escapeValueSuffix)))
                        return false;
                }

                return true;
            };

            const size_t numberOfPackets = (packet.size() / packet.maxSize()) + (packet.size() % packet.maxSize() != 0);

            for (size_t packetIndex = 0; packetIndex < numberOfPackets; packetIndex++)
            {
                const size_t packetStartIndex = packet.maxSize() * packetIndex;
                size_t       packetSize       = 0;

                if (packet.size() > packet.maxSize())
                {
                    if ((packetStartIndex + packet.maxSize()) > packet.size())
                    {
                        packetSize = packet.size() - packetStartIndex;
                    }
                    else
                    {
                        packetSize = packet.maxSize();
                    }
                }
                else
                {
                    packetSize = packet.size();
                }

                if (!writeSingle(channel, static_cast<uint8_t>(USBPacketUpdater::framing_t::boundary), true))
                    return false;

                if (!writeSingle(channel, static_cast<uint8_t>(packet.type())))
                    return false;

                if (!writeSingle(channel, packetSize))
                    return false;

                for (size_t i = 0; i < packetSize; i++)
                {
                    if (!writeSingle(channel, packet[i + packetStartIndex]))
                        return false;
                }
            }

            return true;
        }
    }    // namespace USBOverSerial
}    // namespace Board

#endif