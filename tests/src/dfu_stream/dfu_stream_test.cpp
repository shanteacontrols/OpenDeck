/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/dfu_stream.h"
#include "common/src/dfu_stream/instance/impl/dfu_stream.h"

#include <algorithm>
#include <span>
#include <vector>

namespace
{
    class SinkTest : public opendeck::dfu_stream::Sink
    {
        public:
        bool begin(const opendeck::dfu_stream::Header& header, const uint32_t size) override
        {
            begin_called++;
            accepted_header = header;
            expected_size   = size;
            return begin_result;
        }

        bool write(std::span<const uint8_t> data) override
        {
            write_called++;

            if (!write_result)
            {
                return false;
            }

            payload.insert(payload.end(), data.begin(), data.end());
            return true;
        }

        bool finish() override
        {
            finish_called++;
            return finish_result;
        }

        void abort() override
        {
            abort_called++;
        }

        bool                         begin_result    = true;
        bool                         write_result    = true;
        bool                         finish_result   = true;
        size_t                       begin_called    = 0;
        size_t                       write_called    = 0;
        size_t                       finish_called   = 0;
        size_t                       abort_called    = 0;
        uint32_t                     expected_size   = 0;
        opendeck::dfu_stream::Header accepted_header = {};
        std::vector<uint8_t>         payload;
    };

    opendeck::dfu_stream::StreamStatus feed(opendeck::dfu_stream::DfuStream& parser,
                                            const std::vector<uint8_t>&      stream)
    {
        auto status = opendeck::dfu_stream::StreamStatus::Incomplete;

        for (const auto byte : stream)
        {
            status = parser.feed(byte);
        }

        return status;
    }
}    // namespace

TEST(DfuStream, AcceptsValidStream)
{
    const std::vector<uint8_t>      payload = { 0x10, 0x11, 0x12, 0x13, 0x14 };
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream(payload));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Complete);
    ASSERT_EQ(sink.begin_called, 1);
    ASSERT_EQ(sink.finish_called, 1);
    ASSERT_EQ(sink.abort_called, 0);
    ASSERT_EQ(sink.expected_size, payload.size());
    ASSERT_TRUE(std::equal(sink.accepted_header.begin(), sink.accepted_header.end(), opendeck::tests::dfu_stream::make_stream(payload).begin()));
    ASSERT_EQ(sink.payload, payload);
    ASSERT_EQ(parser.bytes_written(), payload.size());
    ASSERT_EQ(parser.expected_size(), payload.size());
}

TEST(DfuStream, AcceptsStreamInChunks)
{
    const std::vector<uint8_t>      payload = { 0x20, 0x21, 0x22, 0x23 };
    const auto                      stream  = opendeck::tests::dfu_stream::make_stream(payload);
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);

    const auto first = parser.feed(std::span<const uint8_t>(stream.data(), 3));
    const auto rest  = parser.feed(std::span<const uint8_t>(stream.data() + 3, stream.size() - 3));

    ASSERT_EQ(first, opendeck::dfu_stream::StreamStatus::Incomplete);
    ASSERT_EQ(rest, opendeck::dfu_stream::StreamStatus::Complete);
    ASSERT_EQ(sink.payload, payload);
}

TEST(DfuStream, RejectsWrongStartMagic)
{
    auto stream = opendeck::tests::dfu_stream::make_stream({ 0x01, 0x02 });
    stream[0] ^= 0xFF;

    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);

    ASSERT_EQ(parser.feed(stream[0]), opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 0);
    ASSERT_EQ(sink.abort_called, 0);
}

TEST(DfuStream, RejectsUnsupportedFormat)
{
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream({ 0x01 }, OPENDECK_TARGET_UID, opendeck::dfu_stream::FORMAT_VERSION + 1));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 0);
    ASSERT_TRUE(sink.payload.empty());
}

TEST(DfuStream, RejectsWrongTargetUid)
{
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream({ 0x01 }, OPENDECK_TARGET_UID + 1));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 0);
    ASSERT_TRUE(sink.payload.empty());
}

TEST(DfuStream, RejectsEmptyPayload)
{
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream({}));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 0);
    ASSERT_TRUE(sink.payload.empty());
}

TEST(DfuStream, RejectsWrongEndMagicAndAbortsActiveSink)
{
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream({ 0x01, 0x02 }, OPENDECK_TARGET_UID, opendeck::dfu_stream::FORMAT_VERSION, 0));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 1);
    ASSERT_EQ(sink.finish_called, 0);
    ASSERT_EQ(sink.abort_called, 1);
    ASSERT_EQ(sink.payload, (std::vector<uint8_t>{ 0x01, 0x02 }));
}

TEST(DfuStream, RejectsBeginFailure)
{
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);
    sink.begin_result = false;

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream({ 0x01 }));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 1);
    ASSERT_EQ(sink.abort_called, 0);
}

TEST(DfuStream, RejectsWriteFailureAndAbortsActiveSink)
{
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);
    sink.write_result = false;

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream({ 0x01 }));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 1);
    ASSERT_EQ(sink.abort_called, 1);
}

TEST(DfuStream, RejectsFinishFailureAndAbortsActiveSink)
{
    SinkTest                        sink;
    opendeck::dfu_stream::DfuStream parser(sink);
    sink.finish_result = false;

    const auto status = feed(parser, opendeck::tests::dfu_stream::make_stream({ 0x01 }));

    ASSERT_EQ(status, opendeck::dfu_stream::StreamStatus::Invalid);
    ASSERT_EQ(sink.begin_called, 1);
    ASSERT_EQ(sink.finish_called, 1);
    ASSERT_EQ(sink.abort_called, 1);
}
