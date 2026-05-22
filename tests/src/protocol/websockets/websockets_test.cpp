/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/dfu_stream.h"
#include "common/src/dfu_stream/shared/common.h"
#include "common/src/websockets/firmware_upload/firmware_upload.h"
#include "firmware/src/staged_update_writer/hwa/test/hwa_test.h"
#include "firmware/src/staged_update_writer/instance/impl/staged_update_writer.h"

#include <array>
#include <cstring>
#include <span>
#include <vector>

namespace
{
    class FirmwareUploadTest : public ::testing::Test
    {
        protected:
        opendeck::staged_update_writer::HwaTest            hwa;
        opendeck::staged_update_writer::StagedUpdateWriter staged_update_writer = opendeck::staged_update_writer::StagedUpdateWriter(hwa);
        opendeck::websockets::FirmwareUpload               handler              = opendeck::websockets::FirmwareUpload(staged_update_writer);

        static std::array<uint8_t, 1> begin_frame()
        {
            return {
                static_cast<uint8_t>(opendeck::websockets::FirmwareUploadCommand::Begin),
            };
        }

        static std::vector<uint8_t> chunk_frame(std::span<const uint8_t> payload)
        {
            std::vector<uint8_t> frame = {
                static_cast<uint8_t>(opendeck::websockets::FirmwareUploadCommand::Chunk),
            };

            frame.insert(frame.end(), payload.begin(), payload.end());
            return frame;
        }

        static std::array<uint8_t, 1> command_frame(opendeck::websockets::FirmwareUploadCommand command)
        {
            return {
                static_cast<uint8_t>(command),
            };
        }

        static uint32_t ack_bytes_written(const opendeck::websockets::FirmwareUploadAck& response)
        {
            uint32_t bytes_written = 0;
            std::memcpy(&bytes_written, response.data() + 3, sizeof(bytes_written));
            return bytes_written;
        }

        static void expect_ack(const opendeck::websockets::FirmwareUploadAck& response,
                               opendeck::websockets::FirmwareUploadCommand    command,
                               opendeck::websockets::FirmwareUploadStatus     status,
                               uint32_t                                       bytes_written)
        {
            EXPECT_EQ(response.at(0), static_cast<uint8_t>(opendeck::websockets::FirmwareUploadResponse::Ack));
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
        static_cast<uint8_t>(opendeck::websockets::FirmwareUploadCommand::Begin),
        0x01U,
    };

    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Begin, opendeck::websockets::FirmwareUploadStatus::BadRequest, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, BeginsUpload)
{
    const auto frame    = begin_frame();
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Begin, opendeck::websockets::FirmwareUploadStatus::Ok, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, RejectsEmptyChunk)
{
    const auto begin = begin_frame();
    ASSERT_TRUE(handler.handle(begin));

    const auto frame    = command_frame(opendeck::websockets::FirmwareUploadCommand::Chunk);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Chunk, opendeck::websockets::FirmwareUploadStatus::BadRequest, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, WritesChunks)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto dfu   = opendeck::tests::dfu_stream::make_stream(payload);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));

    const auto response = handler.handle(chunk);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Chunk, opendeck::websockets::FirmwareUploadStatus::Ok, payload.size());
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, RejectsIncompleteFinish)
{
    constexpr std::array<uint8_t, 2> payload = {
        0xF0U,
        0xF7U,
    };

    const auto begin = begin_frame();
    const auto chunk = chunk_frame(payload);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(opendeck::websockets::FirmwareUploadCommand::Finish);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Finish, opendeck::websockets::FirmwareUploadStatus::Failed, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, FinishesCompleteUpload)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto dfu   = opendeck::tests::dfu_stream::make_stream(payload);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(opendeck::websockets::FirmwareUploadCommand::Finish);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Finish, opendeck::websockets::FirmwareUploadStatus::Ok, payload.size());
    EXPECT_TRUE(response->finished);
}

TEST_F(FirmwareUploadTest, AbortsUpload)
{
    constexpr std::array<uint8_t, 2> payload = {
        0xF0U,
        0xF7U,
    };

    const auto dfu   = opendeck::tests::dfu_stream::make_stream(payload);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(opendeck::websockets::FirmwareUploadCommand::Abort);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Abort, opendeck::websockets::FirmwareUploadStatus::Ok, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, RejectsDfuTargetMismatch)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto dfu   = opendeck::tests::dfu_stream::make_stream(payload, OPENDECK_TARGET_UID + 1);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));

    const auto response = handler.handle(chunk);

    ASSERT_TRUE(response);
    expect_ack(response->response, opendeck::websockets::FirmwareUploadCommand::Chunk, opendeck::websockets::FirmwareUploadStatus::Failed, 0);
    EXPECT_FALSE(response->finished);
}
