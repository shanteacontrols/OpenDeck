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

#ifdef PROJECT_TARGET_USB_OVER_SERIAL

#include "board/board.h"

namespace board::usb_over_serial
{
    class UsbPacketUpdater
    {
        public:
        UsbPacketUpdater(UsbReadPacket& packet)
            : _packet(packet)
        {}

        enum class appendResult_t : uint8_t
        {
            OK,
            OUT_OF_BOUNDS,
            DONE
        };

        enum class framing_t : uint8_t
        {
            BOUNDARY            = 0x7E,
            ESCAPE              = 0x7D,
            ESCAPE_VALUE_SUFFIX = 0x00
        };

        appendResult_t append(uint8_t value)
        {
            auto onBoundaryFound = [&]()
            {
                // actual boundary value, reset the packet
                _packet.reset();

                //...but also set boundary flag
                _packet._boundaryFound = true;
            };

            if (!_packet._boundaryFound)
            {
                if (value == static_cast<uint8_t>(framing_t::BOUNDARY))
                {
                    _packet._boundaryFound = true;
                }
            }
            else
            {
                if (!_packet._escapeProcessing)
                {
                    if (value == static_cast<uint8_t>(framing_t::ESCAPE))
                    {
                        _packet._escapeProcessing = true;
                    }
                    else if (value == static_cast<uint8_t>(framing_t::BOUNDARY))
                    {
                        onBoundaryFound();
                        return appendResult_t::OK;
                    }
                }
                else
                {
                    _packet._escapeProcessing = false;

                    if (value == static_cast<uint8_t>(framing_t::BOUNDARY))
                    {
                        // this is actual data, do nothing
                    }
                    else if (value == static_cast<uint8_t>(framing_t::ESCAPE_VALUE_SUFFIX))
                    {
                        value = static_cast<uint8_t>(framing_t::ESCAPE);
                    }
                    else
                    {
                        onBoundaryFound();
                        return appendResult_t::OK;
                    }
                }

                if (!_packet._escapeProcessing)
                {
                    if (_packet._type == packetType_t::INVALID)
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
                        if (_packet._count < _packet.MAX_SIZE)
                        {
                            _packet._buffer[_packet._count++] = value;
                        }
                    }
                }

                if (_packet._sizeSet && (_packet._count == _packet._size))
                {
                    _packet._done = true;

                    return appendResult_t::DONE;
                }
            }

            return appendResult_t::OK;
        }

        private:
        UsbReadPacket& _packet;

        bool checkType(uint8_t value)
        {
            switch (value)
            {
            case static_cast<uint8_t>(packetType_t::MIDI):
            case static_cast<uint8_t>(packetType_t::INTERNAL):
                return true;

            default:
                return false;
            }
        }
    };

    bool read(uint8_t channel, UsbReadPacket& packet)
    {
        if (packet.done())
        {
            return true;
        }

        uint8_t value = 0;

        while (board::uart::read(channel, value))
        {
            UsbPacketUpdater updater(packet);

            if (updater.append(value) == UsbPacketUpdater::appendResult_t::DONE)
            {
                return true;
            }
        }

        return false;
    }

    bool write(uint8_t channel, UsbWritePacket& packet)
    {
        auto writeSingle = [](uint8_t channel, uint8_t value, bool initial = false)
        {
            if (!initial && (value == static_cast<uint8_t>(UsbPacketUpdater::framing_t::BOUNDARY)))
            {
                // send escape first
                if (!board::uart::write(channel, static_cast<uint8_t>(UsbPacketUpdater::framing_t::ESCAPE)))
                {
                    return false;
                }
            }

            if (!board::uart::write(channel, value))
            {
                return false;
            }

            if (value == static_cast<uint8_t>(UsbPacketUpdater::framing_t::ESCAPE))
            {
                if (!board::uart::write(channel, static_cast<uint8_t>(UsbPacketUpdater::framing_t::ESCAPE_VALUE_SUFFIX)))
                {
                    return false;
                }
            }

            return true;
        };

        const size_t NUMBER_OF_PACKETS = (packet.size() / packet.maxSize()) + (packet.size() % packet.maxSize() != 0);

        for (size_t packetIndex = 0; packetIndex < NUMBER_OF_PACKETS; packetIndex++)
        {
            const size_t PACKET_START_INDEX = packet.maxSize() * packetIndex;
            size_t       packetSize         = 0;

            if (packet.size() > packet.maxSize())
            {
                if ((PACKET_START_INDEX + packet.maxSize()) > packet.size())
                {
                    packetSize = packet.size() - PACKET_START_INDEX;
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

            if (!writeSingle(channel, static_cast<uint8_t>(UsbPacketUpdater::framing_t::BOUNDARY), true))
            {
                return false;
            }

            if (!writeSingle(channel, static_cast<uint8_t>(packet.type())))
            {
                return false;
            }

            if (!writeSingle(channel, packetSize))
            {
                return false;
            }

            for (size_t i = 0; i < packetSize; i++)
            {
                if (!writeSingle(channel, packet[i + PACKET_START_INDEX]))
                {
                    return false;
                }
            }
        }

        return true;
    }
}    // namespace board::usb_over_serial

#endif