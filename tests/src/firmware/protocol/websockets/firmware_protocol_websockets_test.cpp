/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "common/src/dfu/dfu_stream_parser/writer/test/dfu_writer_test.h"
#include "common/src/dfu/dfu_stream_parser/shared/common.h"
#include "common/src/protocols/websockets/firmware_upload/firmware_upload.h"
#include "firmware/src/dfu/staged_update_writer/hwa/test/hwa_test.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/staged_update_writer.h"

#include <array>
#include <cstring>
#include <span>
#include <vector>

using namespace opendeck;

namespace
{
    class FirmwareUploadTest : public ::testing::Test
    {
        protected:
        firmware::dfu::staged_update_writer::HwaTest            hwa;
        firmware::dfu::staged_update_writer::StagedUpdateWriter staged_update_writer = firmware::dfu::staged_update_writer::StagedUpdateWriter(hwa);
        common::protocols::websockets::FirmwareUpload           handler              = common::protocols::websockets::FirmwareUpload(staged_update_writer);

        static std::array<uint8_t, 1> begin_frame()
        {
            return {
                static_cast<uint8_t>(common::protocols::websockets::FirmwareUploadCommand::Begin),
            };
        }

        static std::vector<uint8_t> chunk_frame(std::span<const uint8_t> payload)
        {
            std::vector<uint8_t> frame = {
                static_cast<uint8_t>(common::protocols::websockets::FirmwareUploadCommand::Chunk),
            };

            frame.insert(frame.end(), payload.begin(), payload.end());
            return frame;
        }

        static std::array<uint8_t, 1> command_frame(common::protocols::websockets::FirmwareUploadCommand command)
        {
            return {
                static_cast<uint8_t>(command),
            };
        }

        static uint32_t ack_bytes_written(const common::protocols::websockets::FirmwareUploadAck& response)
        {
            uint32_t bytes_written = 0;
            std::memcpy(&bytes_written, response.data() + 3, sizeof(bytes_written));
            return bytes_written;
        }

        static void expect_ack(const common::protocols::websockets::FirmwareUploadAck& response,
                               common::protocols::websockets::FirmwareUploadCommand    command,
                               common::protocols::websockets::FirmwareUploadStatus     status,
                               uint32_t                                                bytes_written)
        {
            EXPECT_EQ(response.at(0), static_cast<uint8_t>(common::protocols::websockets::FirmwareUploadResponse::Ack));
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
        static_cast<uint8_t>(common::protocols::websockets::FirmwareUploadCommand::Begin),
        0x01U,
    };

    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Begin, common::protocols::websockets::FirmwareUploadStatus::BadRequest, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, BeginsUpload)
{
    const auto frame    = begin_frame();
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Begin, common::protocols::websockets::FirmwareUploadStatus::Ok, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, RejectsUnsupportedUpload)
{
    common::dfu::dfu_stream_parser::DfuWriterTest unsupported_writer;
    unsupported_writer.supported_result = false;
    common::protocols::websockets::FirmwareUpload unsupported_handler(unsupported_writer);

    const auto frame    = begin_frame();
    const auto response = unsupported_handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Begin, common::protocols::websockets::FirmwareUploadStatus::Unsupported, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(FirmwareUploadTest, RejectsEmptyChunk)
{
    const auto begin = begin_frame();
    ASSERT_TRUE(handler.handle(begin));

    const auto frame    = command_frame(common::protocols::websockets::FirmwareUploadCommand::Chunk);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Chunk, common::protocols::websockets::FirmwareUploadStatus::BadRequest, 0);
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

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));

    const auto response = handler.handle(chunk);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Chunk, common::protocols::websockets::FirmwareUploadStatus::Ok, payload.size());
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

    const auto frame    = command_frame(common::protocols::websockets::FirmwareUploadCommand::Finish);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Finish, common::protocols::websockets::FirmwareUploadStatus::Failed, 0);
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

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(common::protocols::websockets::FirmwareUploadCommand::Finish);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Finish, common::protocols::websockets::FirmwareUploadStatus::Ok, payload.size());
    EXPECT_TRUE(response->finished);
}

TEST_F(FirmwareUploadTest, AbortsUpload)
{
    constexpr std::array<uint8_t, 2> payload = {
        0xF0U,
        0xF7U,
    };

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));
    ASSERT_TRUE(handler.handle(chunk));

    const auto frame    = command_frame(common::protocols::websockets::FirmwareUploadCommand::Abort);
    const auto response = handler.handle(frame);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Abort, common::protocols::websockets::FirmwareUploadStatus::Ok, 0);
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

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload, OPENDECK_TARGET_UID + 1);
    const auto begin = begin_frame();
    const auto chunk = chunk_frame(dfu);
    ASSERT_TRUE(handler.handle(begin));

    const auto response = handler.handle(chunk);

    ASSERT_TRUE(response);
    expect_ack(response->response, common::protocols::websockets::FirmwareUploadCommand::Chunk, common::protocols::websockets::FirmwareUploadStatus::Failed, 0);
    EXPECT_FALSE(response->finished);
}
