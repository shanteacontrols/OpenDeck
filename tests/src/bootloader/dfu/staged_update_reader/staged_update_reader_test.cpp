/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "bootloader/src/dfu/staged_update_reader/hwa/test/hwa_test.h"
#include "bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.h"

#include <optional>
#include <vector>

using namespace opendeck;

namespace
{
    class SinkTest : public opendeck::common::dfu::dfu_stream::Sink
    {
        public:
        bool begin(const opendeck::common::dfu::dfu_stream::Header& header, uint32_t size) override
        {
            begin_count++;
            accepted_header = header;
            expected_size   = size;
            received.clear();
            return begin_result;
        }

        bool write(std::span<const uint8_t> data) override
        {
            if (!write_result)
            {
                return false;
            }

            received.insert(received.end(), data.begin(), data.end());
            return true;
        }

        bool finish() override
        {
            finish_count++;
            return finish_result;
        }

        void abort() override
        {
            abort_count++;
        }

        bool                                      begin_result    = true;
        bool                                      write_result    = true;
        bool                                      finish_result   = true;
        size_t                                    begin_count     = 0;
        size_t                                    finish_count    = 0;
        size_t                                    abort_count     = 0;
        uint32_t                                  expected_size   = 0;
        opendeck::common::dfu::dfu_stream::Header accepted_header = {};
        std::vector<uint8_t>                      received        = {};
    };

    class StagedUpdateReaderTest : public ::testing::Test
    {
        protected:
        static std::vector<uint8_t> payload()
        {
            return {
                0xF0,
                0x00,
                0x53,
                0x43,
                0x01,
                0x02,
                0x03,
                0x04,
                0xF7,
            };
        }

        bootloader::dfu::staged_update_reader::HwaTest            hwa;
        bootloader::dfu::staged_update_reader::StagedUpdateReader reader = bootloader::dfu::staged_update_reader::StagedUpdateReader(hwa);
        SinkTest                                                  sink;
    };
}    // namespace

TEST_F(StagedUpdateReaderTest, IgnoresMissingPendingUpdate)
{
    EXPECT_FALSE(reader.consume(sink));
    EXPECT_TRUE(hwa.init_called());
    EXPECT_EQ(sink.begin_count, 0);
    EXPECT_TRUE(sink.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}

TEST_F(StagedUpdateReaderTest, IgnoresUnavailableStorage)
{
    hwa.set_init_result(false);

    EXPECT_FALSE(reader.consume(sink));
    EXPECT_TRUE(hwa.init_called());
    EXPECT_EQ(sink.begin_count, 0);
    EXPECT_TRUE(sink.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}

TEST_F(StagedUpdateReaderTest, StreamsValidPendingUpdateAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);

    EXPECT_TRUE(reader.consume(sink));
    EXPECT_EQ(sink.begin_count, 1);
    EXPECT_EQ(sink.finish_count, 1);
    EXPECT_EQ(sink.expected_size, data.size());
    EXPECT_EQ(sink.received, data);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_NE(hwa.header_start_magic(), opendeck::common::dfu::dfu_stream::START_COMMAND);
}

TEST_F(StagedUpdateReaderTest, RejectsSinkWriteFailureAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);
    sink.write_result = false;

    EXPECT_FALSE(reader.consume(sink));
    EXPECT_EQ(sink.begin_count, 1);
    EXPECT_EQ(sink.abort_count, 1);
    EXPECT_TRUE(sink.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_NE(hwa.header_start_magic(), opendeck::common::dfu::dfu_stream::START_COMMAND);
}

TEST_F(StagedUpdateReaderTest, RejectsSinkFinishFailureAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);
    sink.finish_result = false;

    EXPECT_FALSE(reader.consume(sink));
    EXPECT_EQ(sink.begin_count, 1);
    EXPECT_EQ(sink.finish_count, 1);
    EXPECT_EQ(sink.abort_count, 1);
    EXPECT_EQ(sink.received, data);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_NE(hwa.header_start_magic(), opendeck::common::dfu::dfu_stream::START_COMMAND);
}

TEST_F(StagedUpdateReaderTest, RejectsInvalidHeaderWithoutClearingAgain)
{
    const auto data = payload();

    hwa.stage(data, 0U);

    EXPECT_FALSE(reader.consume(sink));
    EXPECT_EQ(sink.begin_count, 0);
    EXPECT_TRUE(sink.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
    EXPECT_EQ(hwa.header_start_magic(), 0);
}

TEST_F(StagedUpdateReaderTest, RejectsWrongTargetUid)
{
    const auto data = payload();

    hwa.stage(data, opendeck::common::dfu::dfu_stream::START_COMMAND, opendeck::common::dfu::dfu_stream::FORMAT_VERSION, OPENDECK_TARGET_UID ^ 0x01U);

    EXPECT_FALSE(reader.consume(sink));
    EXPECT_EQ(sink.begin_count, 0);
    EXPECT_TRUE(sink.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}
