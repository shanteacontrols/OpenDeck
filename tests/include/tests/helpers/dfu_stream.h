/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu_stream/shared/common.h"

#include <cstdint>
#include <initializer_list>
#include <span>
#include <vector>

namespace opendeck::tests::dfu_stream
{
    inline void append_u32(std::vector<uint8_t>& data, const uint32_t value)
    {
        constexpr uint32_t BYTE_MASK      = 0xFF;
        constexpr uint8_t  BITS_PER_OCTET = 8;

        for (size_t i = 0; i < sizeof(value); i++)
        {
            data.push_back((value >> (i * BITS_PER_OCTET)) & BYTE_MASK);
        }
    }

    inline std::vector<uint8_t> make_stream(std::span<const uint8_t> payload,
                                            const uint32_t           target_uid     = OPENDECK_TARGET_UID,
                                            const uint32_t           format_version = opendeck::dfu_stream::FORMAT_VERSION,
                                            const uint32_t           end_command    = opendeck::dfu_stream::END_COMMAND)
    {
        std::vector<uint8_t> stream;

        append_u32(stream, opendeck::dfu_stream::START_COMMAND);
        append_u32(stream, format_version);
        append_u32(stream, target_uid);
        append_u32(stream, payload.size());
        stream.insert(stream.end(), payload.begin(), payload.end());
        append_u32(stream, end_command);

        return stream;
    }

    inline std::vector<uint8_t> make_stream(std::initializer_list<uint8_t> payload,
                                            const uint32_t                 target_uid     = OPENDECK_TARGET_UID,
                                            const uint32_t                 format_version = opendeck::dfu_stream::FORMAT_VERSION,
                                            const uint32_t                 end_command    = opendeck::dfu_stream::END_COMMAND)
    {
        return make_stream(std::span<const uint8_t>(payload.begin(), payload.size()), target_uid, format_version, end_command);
    }
}    // namespace opendeck::tests::dfu_stream
