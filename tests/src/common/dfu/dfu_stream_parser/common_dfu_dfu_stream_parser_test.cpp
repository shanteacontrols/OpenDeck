/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "common/src/dfu/dfu_stream_parser/destination/test/destination_test.h"
#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"

#include <algorithm>
#include <span>
#include <vector>

using namespace opendeck;

namespace
{
    using common::dfu::dfu_stream_parser::DestinationTest;

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
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream(payload));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Complete);
    ASSERT_EQ(destination.begin_called, 1);
    ASSERT_EQ(destination.finish_called, 1);
    ASSERT_EQ(destination.abort_called, 0);
    ASSERT_EQ(destination.expected_size, payload.size());
    ASSERT_TRUE(std::equal(destination.accepted_header.begin(), destination.accepted_header.end(), tests::dfu_stream_parser::make_stream(payload).begin()));
    ASSERT_EQ(destination.payload, payload);
    ASSERT_EQ(parser.bytes_written(), payload.size());
    ASSERT_EQ(parser.expected_size(), payload.size());
}

TEST(DfuStreamParser, AcceptsStreamInChunks)
{
    const std::vector<uint8_t>                      payload = { 0x20, 0x21, 0x22, 0x23 };
    const auto                                      stream  = tests::dfu_stream_parser::make_stream(payload);
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);

    const auto first = parser.feed(std::span<const uint8_t>(stream.data(), 3));
    const auto rest  = parser.feed(std::span<const uint8_t>(stream.data() + 3, stream.size() - 3));

    ASSERT_EQ(first, common::dfu::dfu_stream_parser::StreamStatus::Incomplete);
    ASSERT_EQ(rest, common::dfu::dfu_stream_parser::StreamStatus::Complete);
    ASSERT_EQ(destination.payload, payload);
}

TEST(DfuStreamParser, RejectsWrongStartMagic)
{
    auto stream = tests::dfu_stream_parser::make_stream({ 0x01, 0x02 });
    stream[0] ^= 0xFF;

    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);

    ASSERT_EQ(parser.feed(stream[0]), common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 0);
    ASSERT_EQ(destination.abort_called, 0);
}

TEST(DfuStreamParser, RejectsUnsupportedFormat)
{
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }, OPENDECK_TARGET_UID, common::dfu::dfu_stream_parser::FORMAT_VERSION + 1));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 0);
    ASSERT_TRUE(destination.payload.empty());
}

TEST(DfuStreamParser, RejectsWrongTargetUid)
{
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }, OPENDECK_TARGET_UID + 1));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 0);
    ASSERT_TRUE(destination.payload.empty());
}

TEST(DfuStreamParser, RejectsEmptyPayload)
{
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({}));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 0);
    ASSERT_TRUE(destination.payload.empty());
}

TEST(DfuStreamParser, RejectsWrongEndMagicAndAbortsActiveDestination)
{
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01, 0x02 }, OPENDECK_TARGET_UID, common::dfu::dfu_stream_parser::FORMAT_VERSION, 0));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 1);
    ASSERT_EQ(destination.finish_called, 0);
    ASSERT_EQ(destination.abort_called, 1);
    ASSERT_EQ(destination.payload, (std::vector<uint8_t>{ 0x01, 0x02 }));
}

TEST(DfuStreamParser, RejectsBeginFailure)
{
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);
    destination.begin_result = false;

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 1);
    ASSERT_EQ(destination.abort_called, 0);
}

TEST(DfuStreamParser, RejectsWriteFailureAndAbortsActiveDestination)
{
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);
    destination.write_result = false;

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 1);
    ASSERT_EQ(destination.abort_called, 1);
}

TEST(DfuStreamParser, RejectsFinishFailureAndAbortsActiveDestination)
{
    DestinationTest                                 destination;
    common::dfu::dfu_stream_parser::DfuStreamParser parser(destination);
    destination.finish_result = false;

    const auto status = feed(parser, tests::dfu_stream_parser::make_stream({ 0x01 }));

    ASSERT_EQ(status, common::dfu::dfu_stream_parser::StreamStatus::Invalid);
    ASSERT_EQ(destination.begin_called, 1);
    ASSERT_EQ(destination.finish_called, 1);
    ASSERT_EQ(destination.abort_called, 1);
}
