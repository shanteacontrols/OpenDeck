/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_upload.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "common/src/dfu/dfu_stream_parser/shared/common.h"
#include "common/src/dfu/dfu_stream_parser/writer/test/dfu_writer_test.h"
#include "common/src/dfu/upload/instance/impl/upload.h"
#include "common/src/dfu/upload/shared/common.h"

#include <array>
#include <span>
#include <vector>

using namespace opendeck;

namespace
{
    class UploadTest : public ::testing::Test
    {
        protected:
        common::dfu::dfu_stream_parser::DfuWriterTest writer;
        common::dfu::upload::Upload                   upload = common::dfu::upload::Upload(writer);
    };
}    // namespace

TEST_F(UploadTest, IgnoresUnknownFrames)
{
    EXPECT_FALSE(upload.handle(std::span<const uint8_t>{}));

    constexpr std::array<uint8_t, 1> unknown_command = {
        0x7FU,
    };

    EXPECT_FALSE(upload.handle(unknown_command));
}

TEST_F(UploadTest, RejectsMalformedBegin)
{
    constexpr std::array<uint8_t, 2> frame = {
        static_cast<uint8_t>(common::dfu::upload::Command::Begin),
        0x01U,
    };

    const auto response = upload.handle(frame);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Begin, common::dfu::upload::Status::BadRequest, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(UploadTest, BeginsUpload)
{
    const auto frame    = tests::dfu_upload::begin_frame();
    const auto response = upload.handle(frame);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Begin, common::dfu::upload::Status::Ok, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(UploadTest, RejectsUnsupportedUpload)
{
    writer.supported_result = false;

    const auto frame    = tests::dfu_upload::begin_frame();
    const auto response = upload.handle(frame);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Begin, common::dfu::upload::Status::Unsupported, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(UploadTest, RejectsEmptyChunk)
{
    const auto begin = tests::dfu_upload::begin_frame();
    ASSERT_TRUE(upload.handle(begin));

    constexpr std::array<uint8_t, common::dfu::upload::CHUNK_FRAME_OVERHEAD> frame = {
        static_cast<uint8_t>(common::dfu::upload::Command::Chunk),
        0x00U,
        0x00U,
    };
    const auto response = upload.handle(frame);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Chunk, common::dfu::upload::Status::BadRequest, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(UploadTest, WritesChunks)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto begin = tests::dfu_upload::begin_frame();
    const auto chunk = tests::dfu_upload::chunk_frame(dfu);
    ASSERT_TRUE(upload.handle(begin));

    const auto response = upload.handle(chunk);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Chunk, common::dfu::upload::Status::Ok, payload.size());
    EXPECT_FALSE(response->finished);
    EXPECT_EQ(writer.payload(), std::vector<uint8_t>(payload.begin(), payload.end()));
}

TEST_F(UploadTest, RejectsIncompleteFinish)
{
    constexpr std::array<uint8_t, 2> payload = {
        0xF0U,
        0xF7U,
    };

    const auto begin = tests::dfu_upload::begin_frame();
    const auto chunk = tests::dfu_upload::chunk_frame(payload);
    ASSERT_TRUE(upload.handle(begin));
    ASSERT_TRUE(upload.handle(chunk));

    const auto frame    = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);
    const auto response = upload.handle(frame);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Finish, common::dfu::upload::Status::Failed, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(UploadTest, FinishesCompleteUpload)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto begin = tests::dfu_upload::begin_frame();
    const auto chunk = tests::dfu_upload::chunk_frame(dfu);
    ASSERT_TRUE(upload.handle(begin));
    ASSERT_TRUE(upload.handle(chunk));

    const auto frame    = tests::dfu_upload::command_frame(common::dfu::upload::Command::Finish);
    const auto response = upload.handle(frame);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Finish, common::dfu::upload::Status::Ok, payload.size());
    EXPECT_TRUE(response->finished);
    EXPECT_EQ(writer.finish_called, 1U);
}

TEST_F(UploadTest, AbortsUpload)
{
    constexpr std::array<uint8_t, 2> payload = {
        0xF0U,
        0xF7U,
    };

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload);
    const auto begin = tests::dfu_upload::begin_frame();
    const auto chunk = tests::dfu_upload::chunk_frame(dfu);
    ASSERT_TRUE(upload.handle(begin));
    ASSERT_TRUE(upload.handle(chunk));

    const auto frame    = tests::dfu_upload::command_frame(common::dfu::upload::Command::Abort);
    const auto response = upload.handle(frame);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Abort, common::dfu::upload::Status::Ok, 0);
    EXPECT_FALSE(response->finished);
    EXPECT_EQ(writer.abort_called, 1U);
}

TEST_F(UploadTest, RejectsDfuTargetMismatch)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto dfu   = tests::dfu_stream_parser::make_stream(payload, OPENDECK_TARGET_UID + 1);
    const auto begin = tests::dfu_upload::begin_frame();
    const auto chunk = tests::dfu_upload::chunk_frame(dfu);
    ASSERT_TRUE(upload.handle(begin));

    const auto response = upload.handle(chunk);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Chunk, common::dfu::upload::Status::Failed, 0);
    EXPECT_FALSE(response->finished);
}

TEST_F(UploadTest, RejectsConcatenatedChunkFrames)
{
    constexpr std::array<uint8_t, 4> payload = {
        0xF0U,
        0x00U,
        0x53U,
        0xF7U,
    };

    const auto dfu          = tests::dfu_stream_parser::make_stream(payload);
    const auto begin        = tests::dfu_upload::begin_frame();
    const auto first        = tests::dfu_upload::chunk_frame(dfu);
    const auto second_start = tests::dfu_upload::command_frame(common::dfu::upload::Command::Chunk);
    ASSERT_TRUE(upload.handle(begin));

    std::vector<uint8_t> concatenated = first;
    concatenated.insert(concatenated.end(), second_start.begin(), second_start.end());

    const auto response = upload.handle(concatenated);

    ASSERT_TRUE(response);
    tests::dfu_upload::expect_ack(response->response, common::dfu::upload::Command::Chunk, common::dfu::upload::Status::BadRequest, 0);
    EXPECT_FALSE(response->finished);
}
