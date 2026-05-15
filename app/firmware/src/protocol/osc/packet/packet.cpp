/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/protocol/osc/packet/internal.h"

#include "zlibs/utils/misc/numeric.h"

#include <zephyr/sys/byteorder.h>

#include <algorithm>
#include <cstring>

using namespace opendeck::protocol::osc;

namespace zmisc = zlibs::utils::misc;

namespace
{
    /** @brief OSC strings and payload fields are padded to 4-byte boundaries. */
    constexpr size_t ALIGNMENT = 4;

    /**
     * @brief Reads one padded OSC string from a packet.
     *
     * @param packet Packet bytes being decoded.
     * @param offset Current read offset, advanced past the padded string.
     * @param value String view pointing into packet storage.
     *
     * @return `true` when a valid string was decoded.
     */
    bool read_string(std::span<const uint8_t> packet, size_t& offset, std::string_view& value)
    {
        if (offset >= packet.size())
        {
            return false;
        }

        const size_t start = offset;

        while ((offset < packet.size()) && (packet[offset] != 0))
        {
            offset++;
        }

        if (offset >= packet.size())
        {
            return false;
        }

        value = std::string_view(reinterpret_cast<const char*>(packet.data() + start), offset - start);
        offset++;

        while ((offset % ALIGNMENT) != 0U)
        {
            offset++;
        }

        return offset <= packet.size();
    }

    /**
     * @brief Reads one big-endian OSC int32 argument from a packet.
     *
     * @param packet Packet bytes being decoded.
     * @param offset Current read offset, advanced past the integer.
     * @param value Integer output value.
     *
     * @return `true` when an integer was decoded.
     */
    bool read_int32(std::span<const uint8_t> packet, size_t& offset, int32_t& value)
    {
        if ((offset > packet.size()) || ((packet.size() - offset) < sizeof(uint32_t)))
        {
            return false;
        }

        uint32_t encoded = 0;
        memcpy(&encoded, packet.data() + offset, sizeof(encoded));
        offset += sizeof(encoded);

        value = static_cast<int32_t>(sys_be32_to_cpu(encoded));
        return true;
    }

    /**
     * @brief Reads one big-endian OSC float32 argument from a packet.
     *
     * @param packet Packet bytes being decoded.
     * @param offset Current read offset, advanced past the float.
     * @param value Float output value.
     *
     * @return `true` when a float was decoded.
     */
    bool read_float32(std::span<const uint8_t> packet, size_t& offset, float& value)
    {
        if ((offset > packet.size()) || ((packet.size() - offset) < sizeof(uint32_t)))
        {
            return false;
        }

        uint32_t encoded = 0;
        memcpy(&encoded, packet.data() + offset, sizeof(encoded));
        offset += sizeof(encoded);

        const uint32_t raw = sys_be32_to_cpu(encoded);
        static_assert(sizeof(raw) == sizeof(value));
        memcpy(&value, &raw, sizeof(value));

        return true;
    }
}    // namespace

internal::PacketWriter::PacketWriter(std::span<uint8_t> buffer)
    : _buffer(buffer)
{}

bool internal::PacketWriter::add_address(std::string_view value)
{
    return add_string(value);
}

bool internal::PacketWriter::add_address(OscIndexedAddress address)
{
    return add_bytes(address.prefix) &&
           add_byte('/') &&
           add_decimal(address.index) &&
           add_byte(0) &&
           pad();
}

bool internal::PacketWriter::add_type_tags(std::string_view value)
{
    return add_string(value);
}

size_t internal::PacketWriter::size() const
{
    return _offset;
}

bool internal::PacketWriter::add_string(std::string_view value)
{
    if ((_offset + value.size() + 1) > _buffer.size())
    {
        return false;
    }

    return add_bytes(value) &&
           add_byte(0) &&
           pad();
}

bool internal::PacketWriter::add_bytes(std::string_view value)
{
    if ((_offset + value.size()) > _buffer.size())
    {
        return false;
    }

    memcpy(_buffer.data() + _offset, value.data(), value.size());
    _offset += value.size();

    return true;
}

bool internal::PacketWriter::add_byte(uint8_t value)
{
    if (_offset >= _buffer.size())
    {
        return false;
    }

    _buffer[_offset++] = value;

    return true;
}

bool internal::PacketWriter::add_decimal(size_t value)
{
    if (value >= zmisc::DECIMAL_BASE)
    {
        if (!add_decimal(value / zmisc::DECIMAL_BASE))
        {
            return false;
        }
    }

    return add_byte(static_cast<uint8_t>('0' + (value % zmisc::DECIMAL_BASE)));
}

bool internal::PacketWriter::pad()
{
    while ((_offset % ALIGNMENT) != 0U)
    {
        if (!add_byte(0))
        {
            return false;
        }
    }

    return true;
}

bool internal::PacketWriter::add_int32(int32_t value)
{
    if ((_offset > _buffer.size()) || ((_buffer.size() - _offset) < sizeof(uint32_t)))
    {
        return false;
    }

    const uint32_t encoded = sys_cpu_to_be32(static_cast<uint32_t>(value));

    memcpy(_buffer.data() + _offset, &encoded, sizeof(encoded));
    _offset += sizeof(encoded);

    return true;
}

bool internal::PacketWriter::add_float32(float value)
{
    if ((_offset > _buffer.size()) || ((_buffer.size() - _offset) < sizeof(uint32_t)))
    {
        return false;
    }

    uint32_t raw = 0;
    static_assert(sizeof(raw) == sizeof(value));
    memcpy(&raw, &value, sizeof(raw));

    const uint32_t encoded = sys_cpu_to_be32(raw);

    memcpy(_buffer.data() + _offset, &encoded, sizeof(encoded));
    _offset += sizeof(encoded);

    return true;
}

OscMessageView::OscMessageView(std::span<const uint8_t> packet,
                               std::string_view         address,
                               std::string_view         type_tags,
                               std::span<const size_t>  arg_offsets)
    : _packet(packet)
    , _address(address)
    , _type_tags(type_tags)
    , _arg_count(arg_offsets.size())
{
    std::copy(arg_offsets.begin(), arg_offsets.end(), _arg_offsets.begin());
}

std::string_view OscMessageView::address() const
{
    return _address;
}

std::string_view OscMessageView::type_tags() const
{
    return _type_tags;
}

bool OscMessageView::empty() const
{
    return _arg_count == 0;
}

bool OscMessageView::arg_matches(size_t index, char type_tag) const
{
    return (index < _arg_count) &&
           ((index + 1U) < _type_tags.size()) &&
           (_type_tags[index + 1U] == type_tag);
}

std::optional<int32_t> OscMessageView::read_arg(size_t index, OscInt32) const
{
    if (!arg_matches(index, OscInt32::TYPE_TAG))
    {
        return {};
    }

    int32_t value  = 0;
    size_t  offset = _arg_offsets[index];

    if (!read_int32(_packet, offset, value))
    {
        return {};
    }

    return value;
}

std::optional<std::string_view> OscMessageView::read_arg(size_t index, OscString) const
{
    if (!arg_matches(index, OscString::TYPE_TAG))
    {
        return {};
    }

    std::string_view value  = {};
    size_t           offset = _arg_offsets[index];

    if (!read_string(_packet, offset, value))
    {
        return {};
    }

    return value;
}

std::optional<float> OscMessageView::read_arg(size_t index, OscFloat32) const
{
    if (!arg_matches(index, OscFloat32::TYPE_TAG))
    {
        return {};
    }

    float  value  = 0.0F;
    size_t offset = _arg_offsets[index];

    if (!read_float32(_packet, offset, value))
    {
        return {};
    }

    return value;
}

std::optional<OscMessageView> opendeck::protocol::osc::parse_message(std::span<const uint8_t> packet)
{
    std::array<size_t, MAX_ARGUMENT_COUNT> arg_offsets = {};
    size_t                                 offset      = 0;
    std::string_view                       address     = {};
    std::string_view                       type_tags   = {};

    if (!read_string(packet, offset, address) ||
        !read_string(packet, offset, type_tags))
    {
        return {};
    }

    if (type_tags.empty() || (type_tags.front() != ','))
    {
        return {};
    }

    const size_t arg_count = type_tags.size() - 1U;

    if (arg_count > arg_offsets.size())
    {
        return {};
    }

    for (size_t i = 0; i < arg_count; i++)
    {
        arg_offsets[i] = offset;

        switch (type_tags[i + 1U])
        {
        case OscInt32::TYPE_TAG:
        {
            int32_t ignored = 0;

            if (!read_int32(packet, offset, ignored))
            {
                return {};
            }

            break;
        }

        case OscString::TYPE_TAG:
        {
            std::string_view ignored = {};

            if (!read_string(packet, offset, ignored))
            {
                return {};
            }

            break;
        }

        case OscFloat32::TYPE_TAG:
        {
            float ignored = 0.0F;

            if (!read_float32(packet, offset, ignored))
            {
                return {};
            }

            break;
        }

        default:
            return {};
        }
    }

    if (offset != packet.size())
    {
        return {};
    }

    return OscMessageView(packet,
                          address,
                          type_tags,
                          std::span<const size_t>(arg_offsets.data(), arg_count));
}
