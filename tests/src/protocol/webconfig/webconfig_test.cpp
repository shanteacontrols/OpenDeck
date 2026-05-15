/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "firmware/src/protocol/webconfig/firmware_upload/firmware_upload.h"
#include "firmware/src/staged_update/hwa_test.h"
#include "firmware/src/staged_update/staged_update.h"

#include <array>
#include <cstring>
#include <span>
#include <vector>

using namespace opendeck::protocol::webconfig;

namespace
{
    class FirmwareUploadTest : public ::testing::Test
    {
        protected:
        opendeck::staged_update::HwaTest      hwa;
        opendeck::staged_update::StagedUpdate staged_update = opendeck::staged_update::StagedUpdate(hwa);
        FirmwareUploadHandler                 handler       = FirmwareUploadHandler(staged_update);

        static std::vector<uint8_t> begin_frame(uint32_t size)
        {
            std::vector<uint8_t> frame = {
                static_cast<uint8_t>(FirmwareUploadCommand::Begin),
                0,
                0,
                0,
                0,
            };

            std::memcpy(frame.data() + 1, &size, sizeof(size));
            return frame;
        }

        static std::vector<uint8_t> chunk_frame(std::span<const uint8_t> payload)
        {
            std::vector<uint8_t> frame = {
                static_cast<uint8_t>(FirmwareUploadCommand::Chunk),
            };

            frame.insert(frame.end(), payload.begin(), payload.end());
            return frame;
        }

        static std::array<uint8_t, 1> command_frame(FirmwareUploadCommand command)
        {
            return {
                static_cast<uint8_t>(command),
            };
        }

        static uint32_t ack_bytes_written(const FirmwareUploadHandler::Response& response)
        {
            uint32_t bytes_written = 0;
            std::memcpy(&bytes_written, response.data() + 3, sizeof(bytes_written));
            return bytes_written;
        }

        static void expect_ack(const FirmwareUploadHandler::Response& response,
                               FirmwareUploadCommand                  command,
                               FirmwareUploadStatus                   status,
                               uint32_t                               bytes_written)
        {
            EXPECT_EQ(response.at(0), static_cast<uint8_t>(FirmwareUploadResponse::Ack));
            EXPECT_EQ(response.at(1), static_cast<uint8_t>(command));
            EXPECT_EQ(response.at(2), static_cast<uint8_t>(status));
            EXPECT_EQ(ack_bytes_written(response), bytes_written);
        }
    };
}    // namespace

TEST_F(FirmwareUploadTest, IgnoresUnknownFrames)
{
    EXPECT_FALSE(handler.handle(std::span<const uint8_t>{}));

    constexpr std::array<uint8_t, 1> unknown_command = {
        0x7FU,
    };

    EXPECT_FALSE(handler.handle(unknown_command));
}

TEST_F(FirmwareUploadTest, RejectsMalformedBegin)
{
    constexpr std::array<uint8_t, 2> frame = {
        static_cast<uint8_t>(FirmwareUploadCommand::Begin),
        0x01U,
    };

    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(*response, FirmwareUploadCommand::Begin, FirmwareUploadStatus::BadRequest, 0);
}

TEST_F(FirmwareUploadTest, BeginsUpload)
{
    const auto frame    = begin_frame(16);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(*response, FirmwareUploadCommand::Begin, FirmwareUploadStatus::Ok, 0);
}

TEST_F(FirmwareUploadTest, RejectsEmptyChunk)
{
    const auto begin = begin_frame(4);
    ASSERT_TRUE(handler.handle(begin));

    const auto frame    = command_frame(FirmwareUploadCommand::Chunk);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(*response, FirmwareUploadCommand::Chunk, FirmwareUploadStatus::BadRequest, 0);
}

TEST_F(FirmwareUploadTest, WritesChunks)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto begin = begin_frame(payload.size());
    const auto chunk = chunk_frame(payload);
    ASSERT_TRUE(handler.handle(begin));

    const auto response = handler.handle(chunk);

    ASSERT_TRUE(response);
    expect_ack(*response, FirmwareUploadCommand::Chunk, FirmwareUploadStatus::Ok, payload.size());
}

TEST_F(FirmwareUploadTest, RejectsIncompleteFinish)
{
    constexpr std::array<uint8_t, 2> payload = {
        0xF0U,
        0xF7U,
    };

    const auto begin = begin_frame(4);
    const auto chunk = chunk_frame(payload);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(FirmwareUploadCommand::Finish);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(*response, FirmwareUploadCommand::Finish, FirmwareUploadStatus::Failed, 0);
    EXPECT_FALSE(handler.take_reboot_request());
}

TEST_F(FirmwareUploadTest, FinishesCompleteUpload)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto begin = begin_frame(payload.size());
    const auto chunk = chunk_frame(payload);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(FirmwareUploadCommand::Finish);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(*response, FirmwareUploadCommand::Finish, FirmwareUploadStatus::Ok, payload.size());
    EXPECT_TRUE(handler.take_reboot_request());
    EXPECT_FALSE(handler.take_reboot_request());
}

TEST_F(FirmwareUploadTest, AbortsUpload)
{
    constexpr std::array<uint8_t, 2> payload = {
        0xF0U,
        0xF7U,
    };

    const auto begin = begin_frame(payload.size());
    const auto chunk = chunk_frame(payload);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(FirmwareUploadCommand::Abort);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(*response, FirmwareUploadCommand::Abort, FirmwareUploadStatus::Ok, 0);
    EXPECT_EQ(staged_update.bytes_written(), 0);
}
