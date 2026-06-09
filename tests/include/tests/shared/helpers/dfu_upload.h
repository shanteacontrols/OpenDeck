/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "tests/shared/common.h"
#include "common/src/dfu/upload/shared/common.h"

#include <cstdint>
#include <span>
#include <vector>

namespace opendeck::tests::dfu_upload
{
    inline std::vector<uint8_t> command_frame(opendeck::common::dfu::upload::Command command,
                                              std::span<const uint8_t>               payload = {})
    {
        std::vector<uint8_t> frame = {
            static_cast<uint8_t>(command),
        };

        if (command == opendeck::common::dfu::upload::Command::Chunk)
        {
            frame.push_back(static_cast<uint8_t>(payload.size() & 0xFFU));
            frame.push_back(static_cast<uint8_t>((payload.size() >> 8U) & 0xFFU));
        }

        frame.insert(frame.end(), payload.begin(), payload.end());

        return frame;
    }

    inline std::vector<uint8_t> chunk_frame(std::span<const uint8_t> payload)
    {
        return command_frame(opendeck::common::dfu::upload::Command::Chunk, payload);
    }

    inline std::vector<uint8_t> begin_frame()
    {
        return command_frame(opendeck::common::dfu::upload::Command::Begin);
    }

    inline void expect_ack(const opendeck::common::dfu::upload::Ack& response,
                           opendeck::common::dfu::upload::Command    command,
                           opendeck::common::dfu::upload::Status     status,
                           uint32_t                                  bytes_written)
    {
        EXPECT_EQ(response.response, opendeck::common::dfu::upload::Response::Ack);
        EXPECT_EQ(response.command, command);
        EXPECT_EQ(response.status, status);
        EXPECT_EQ(response.bytes_written, bytes_written);
    }

    inline void expect_ack(const std::vector<uint8_t>&            response,
                           opendeck::common::dfu::upload::Command command,
                           opendeck::common::dfu::upload::Status  status,
                           uint32_t                               bytes_written)
    {
        ASSERT_EQ(response.size(), sizeof(opendeck::common::dfu::upload::Ack));

        const auto ack = opendeck::common::dfu::upload::ack_from_bytes(response);

        ASSERT_TRUE(ack);
        expect_ack(*ack, command, status, bytes_written);
    }
}    // namespace opendeck::tests::dfu_upload
