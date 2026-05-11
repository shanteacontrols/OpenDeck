/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>

namespace opendeck::protocol::osc::internal
{
    /**
     * @brief Writes OSC packet fields into a fixed packet buffer.
     */
    class PacketWriter
    {
        public:
        /**
         * @brief Creates a writer which appends OSC data into the provided packet buffer.
         */
        explicit PacketWriter(std::span<uint8_t> buffer);

        /**
         * @brief Appends the OSC address.
         */
        bool add_address(std::string_view value);

        /**
         * @brief Appends the OSC address with a numeric component index suffix.
         */
        bool add_address(OscIndexedAddress address);

        /**
         * @brief Appends the OSC type-tag string.
         */
        bool add_type_tags(std::string_view value);

        /**
         * @brief Appends one typed OSC argument.
         */
        template<typename Arg>
        bool add_arg(const Arg& arg)
        {
            if constexpr (std::is_same_v<Arg, OscInt32>)
            {
                return add_int32(arg.value);
            }

            if constexpr (std::is_same_v<Arg, OscString>)
            {
                return add_string(arg.value);
            }

            if constexpr (std::is_same_v<Arg, OscFloat32>)
            {
                return add_float32(arg.value);
            }

            static_assert(std::is_same_v<Arg, OscInt32> ||
                              std::is_same_v<Arg, OscString> ||
                              std::is_same_v<Arg, OscFloat32>,
                          "Unsupported OSC argument type");

            return false;
        }

        /**
         * @brief Returns the number of packet bytes written so far.
         */
        size_t size() const;

        private:
        /**
         * @brief Appends a null-terminated OSC string and pads it to a 4-byte boundary.
         */
        bool add_string(std::string_view value);

        /**
         * @brief Appends raw bytes without OSC string padding.
         */
        bool add_bytes(std::string_view value);

        /**
         * @brief Appends a raw byte without OSC string padding.
         */
        bool add_byte(uint8_t value);

        /**
         * @brief Appends a decimal number without OSC string padding.
         */
        bool add_decimal(size_t value);

        /**
         * @brief Pads the packet to the next OSC 4-byte boundary.
         */
        bool pad();

        /**
         * @brief Appends one big-endian 32-bit integer OSC argument.
         */
        bool add_int32(int32_t value);

        /**
         * @brief Appends one big-endian 32-bit floating-point OSC argument.
         */
        bool add_float32(float value);

        std::span<uint8_t> _buffer;
        size_t             _offset = 0;
    };

    /**
     * @brief Builds one OSC packet using the provided address representation.
     *
     * @param packet Packet buffer to fill.
     * @param address OSC address string or indexed address descriptor.
     * @param args OSC arguments to append.
     *
     * @return Number of bytes written, or empty if encoding does not fit.
     */
    template<typename Address, typename... Args>
    std::optional<size_t> make_packet_impl(PacketBuffer& packet, Address address, const Args&... args)
    {
        packet.fill(0);

        PacketWriter writer(packet);

        if (!writer.add_address(address))
        {
            return {};
        }

        std::array<char, sizeof...(Args) + 2> type_tags = {
            ',',
            Args::TYPE_TAG...,
            '\0',
        };

        if (!writer.add_type_tags(std::string_view(type_tags.data(), type_tags.size() - 1)))
        {
            return {};
        }

        if (!(writer.add_arg(args) && ...))
        {
            return {};
        }

        return writer.size();
    }
}    // namespace opendeck::protocol::osc::internal
