/*

Copyright 2015-2022 Igor Petrovic

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

#include <cinttypes>
#include <cstddef>

namespace board::usbOverSerial
{
    class USBPacketUpdater;

    enum class packetType_t : uint8_t
    {
        INVALID,     ///< Placeholder type used to indicate that the packet type isn't set.
        MIDI,        ///< MIDI packet in OpenDeck format.
        INTERNAL,    ///< Internal command used for target MCU <> USB link communication.
        CDC,         ///< CDC packet in OpenDeck format.
    };

    enum class internalCMD_t : uint8_t
    {
        REBOOT_BTLDR,
        USB_STATE,
        BAUDRATE_CHANGE,
        UNIQUE_ID,
        DISCONNECT_USB,
        CONNECT_USB,
        LINK_READY,
        FACTORY_RESET
    };

    class USBPacketBase
    {
        public:
        virtual ~USBPacketBase() = default;

        virtual packetType_t type() const                   = 0;
        virtual uint8_t*     buffer() const                 = 0;
        virtual size_t       size() const                   = 0;
        virtual size_t       maxSize() const                = 0;
        virtual uint8_t      operator[](size_t index) const = 0;
    };

    class USBReadPacket : public USBPacketBase
    {
        public:
        USBReadPacket(uint8_t* data, size_t maxSize)
            : _buffer(data)
            , MAX_SIZE(maxSize)
        {}

        packetType_t type() const override
        {
            return _type;
        }

        uint8_t* buffer() const override
        {
            return _buffer;
        }

        size_t size() const override
        {
            return _size;
        }

        size_t maxSize() const override
        {
            return MAX_SIZE;
        }

        uint8_t operator[](size_t index) const override
        {
            if (index < _size)
            {
                return _buffer[index];
            }

            return 0;
        }

        bool done() const
        {
            return _done;
        }

        void reset()
        {
            _type             = packetType_t::INVALID;
            _size             = 0;
            _sizeSet          = false;
            _count            = 0;
            _boundaryFound    = false;
            _escapeProcessing = false;
            _done             = false;
        }

        private:
        packetType_t _type             = packetType_t::INVALID;
        uint8_t*     _buffer           = nullptr;
        size_t       _size             = 0;
        bool         _sizeSet          = 0;
        size_t       _count            = 0;
        bool         _boundaryFound    = false;
        bool         _escapeProcessing = false;
        bool         _done             = false;
        const size_t MAX_SIZE;

        friend class USBPacketUpdater;
    };

    class USBWritePacket : public USBPacketBase
    {
        public:
        USBWritePacket(packetType_t type, uint8_t* data, size_t size, size_t maxSize)
            : _type(type)
            , _buffer(data)
            , SIZE(size)
            , MAX_SIZE(maxSize)
        {}

        packetType_t type() const override
        {
            return _type;
        }

        uint8_t* buffer() const override
        {
            return _buffer;
        }

        size_t size() const override
        {
            return SIZE;
        }

        size_t maxSize() const override
        {
            return MAX_SIZE;
        }

        uint8_t operator[](size_t index) const override
        {
            if (index < SIZE)
            {
                return _buffer[index];
            }

            return 0;
        }

        private:
        packetType_t _type   = packetType_t::MIDI;
        uint8_t*     _buffer = nullptr;
        const size_t SIZE;
        const size_t MAX_SIZE;
    };

    /// Used to read data using custom OpenDeck format from UART interface.
    /// param [in]: channel         UART channel on MCU.
    /// param [in]: packet          Reference to structure in which read data is being stored.
    /// returns: True if data is available, false otherwise.
    bool read(uint8_t channel, USBReadPacket& packet);

    /// Used to write data using custom OpenDeck format to UART interface.
    /// param [in]: channel         UART channel on MCU.
    /// param [in]: packet          Reference to structure in which data to write is stored.
    /// returns: True on success, false otherwise.
    bool write(uint8_t channel, USBWritePacket& packet);
}    // namespace board::usbOverSerial