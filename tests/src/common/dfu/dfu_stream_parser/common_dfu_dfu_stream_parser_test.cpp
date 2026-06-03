/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "common/src/dfu/dfu_stream_parser/writer/test/dfu_writer_test.h"
#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"

#include <algorithm>
#include <span>
#include <vector>

using namespace opendeck;

namespace
{
    using common::dfu::dfu_stream_parser::DfuWriterTest;

    common::dfu::dfu_stream_parser::StreamStatus feed(common::dfu::dfu_stream_parser::DfuStreamParser& parser,
                                                      const std::vector<uint8_t>&                      stream)
    {
        auto status = common::dfu::dfu_stream_parser::StreamStatus::Incomplete;

        for (const auto byte : stream)
        {
            status = parser.feed(byte);
        }

        return status;
    }
}    // namespace

TEST(DfuStreamParser, AcceptsValidStream)
{
    const std::vector<uint8_t>                      payload = { 0x10, 0x11, 0x12, 0x13, 0x14 };
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream(payload));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Complete);
    ASSERT_EQ(writer.begin_called, 1);
    ASSERT_EQ(writer.finish_called, 1);
    ASSERT_EQ(writer.abort_called, 0);
    ASSERT_EQ(writer.expected_size, payload.size());
    ASSERT_TRUE(std::equal(writer.accepted_header.begin(), writer.accepted_header.end(), tests::dfu_stream_parser::make_stream(payload).begin()));
    ASSERT_EQ(writer.payload(), payload);
    ASSERT_EQ(parser.bytes_written(), payload.size());
    ASSERT_EQ(parser.expected_size(), payload.size());
}

TEST(DfuStreamParser, AcceptsStreamInChunks)
{
    const std::vector<uint8_t>                      payload = { 0x20, 0x21, 0x22, 0x23 };
    const auto                                      stream  = tests::dfu_stream_parser::make_stream(payload);
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);

    const auto first = parser.feed(std::span<const uint8_t>(stream.data(), 3));
    const auto rest  = parser.feed(std::span<const uint8_t>(stream.data() + 3, stream.size() - 3));

    ASSERT_EQ(first, common::dfu::dfu_stream_parser::StreamStatus::Incomplete);
    ASSERT_EQ(rest, common::dfu::dfu_stream_parser::StreamStatus::Complete);
    ASSERT_EQ(writer.payload(), payload);
}

TEST(DfuStreamParser, RejectsWrongStartMagic)
{
    auto stream = tests::dfu_stream_parser::make_stream({ 0x01, 0x02 });
    stream[0] ^= 0xFF;

    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);

    ASSERT_EQ(parser.feed(stream[0]), common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 0);
    ASSERT_EQ(writer.abort_called, 0);
}

TEST(DfuStreamParser, RejectsUnsupportedFormat)
{
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }, OPENDECK_TARGET_UID, common::dfu::dfu_stream_parser::FORMAT_VERSION + 1));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 0);
    ASSERT_TRUE(writer.payload().empty());
}

TEST(DfuStreamParser, RejectsWrongTargetUid)
{
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }, OPENDECK_TARGET_UID + 1));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 0);
    ASSERT_TRUE(writer.payload().empty());
}

TEST(DfuStreamParser, RejectsEmptyPayload)
{
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({}));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 0);
    ASSERT_TRUE(writer.payload().empty());
}

TEST(DfuStreamParser, RejectsWrongEndMagicAndAbortsActiveWriter)
{
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01, 0x02 }, OPENDECK_TARGET_UID, common::dfu::dfu_stream_parser::FORMAT_VERSION, 0));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 1);
    ASSERT_EQ(writer.finish_called, 0);
    ASSERT_EQ(writer.abort_called, 1);
    ASSERT_TRUE(writer.payload().empty());
}

TEST(DfuStreamParser, RejectsBeginFailure)
{
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);
    writer.begin_result = false;

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 1);
    ASSERT_EQ(writer.abort_called, 0);
}

TEST(DfuStreamParser, RejectsWriteFailureAndAbortsActiveWriter)
{
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);
    writer.fail_write();

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 1);
    ASSERT_EQ(writer.abort_called, 1);
}

TEST(DfuStreamParser, RejectsFinishFailureAndAbortsActiveWriter)
{
    DfuWriterTest                                   writer;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(writer);
    writer.finish_result = false;

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(writer.begin_called, 1);
    ASSERT_EQ(writer.finish_called, 1);
    ASSERT_EQ(writer.abort_called, 1);
}
