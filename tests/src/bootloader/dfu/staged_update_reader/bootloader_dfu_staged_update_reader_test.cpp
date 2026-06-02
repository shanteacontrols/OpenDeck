/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "common/src/dfu/dfu_stream_parser/destination/test/destination_test.h"
#include "bootloader/src/dfu/staged_update_reader/hwa/test/hwa_test.h"
#include "bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.h"

#include <optional>
#include <vector>

using namespace opendeck;

namespace
{
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
        common::dfu::dfu_stream_parser::DestinationTest           consumer_destination;
    };
}    // namespace

TEST_F(StagedUpdateReaderTest, IgnoresMissingPendingUpdate)
{
    EXPECT_FALSE(reader.consume(consumer_destination));
    EXPECT_TRUE(hwa.init_called());
    EXPECT_EQ(consumer_destination.begin_called, 0);
    EXPECT_TRUE(consumer_destination.payload.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}

TEST_F(StagedUpdateReaderTest, IgnoresUnavailableStorage)
{
    hwa.set_init_result(false);

    EXPECT_FALSE(reader.consume(consumer_destination));
    EXPECT_TRUE(hwa.init_called());
    EXPECT_EQ(consumer_destination.begin_called, 0);
    EXPECT_TRUE(consumer_destination.payload.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}

TEST_F(StagedUpdateReaderTest, StreamsValidPendingUpdateAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);

    EXPECT_TRUE(reader.consume(consumer_destination));
    EXPECT_EQ(consumer_destination.begin_called, 1);
    EXPECT_EQ(consumer_destination.finish_called, 1);
    EXPECT_EQ(consumer_destination.expected_size, data.size());
    EXPECT_EQ(consumer_destination.payload, data);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_NE(hwa.header_start_magic(), common::dfu::dfu_stream_parser::START_COMMAND);
}

TEST_F(StagedUpdateReaderTest, RejectsDestinationWriteFailureAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);
    consumer_destination.write_result = false;

    EXPECT_FALSE(reader.consume(consumer_destination));
    EXPECT_EQ(consumer_destination.begin_called, 1);
    EXPECT_EQ(consumer_destination.abort_called, 1);
    EXPECT_TRUE(consumer_destination.payload.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_NE(hwa.header_start_magic(), common::dfu::dfu_stream_parser::START_COMMAND);
}

TEST_F(StagedUpdateReaderTest, RejectsDestinationFinishFailureAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);
    consumer_destination.finish_result = false;

    EXPECT_FALSE(reader.consume(consumer_destination));
    EXPECT_EQ(consumer_destination.begin_called, 1);
    EXPECT_EQ(consumer_destination.finish_called, 1);
    EXPECT_EQ(consumer_destination.abort_called, 1);
    EXPECT_EQ(consumer_destination.payload, data);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_NE(hwa.header_start_magic(), common::dfu::dfu_stream_parser::START_COMMAND);
}

TEST_F(StagedUpdateReaderTest, RejectsInvalidHeaderWithoutClearingAgain)
{
    const auto data = payload();

    hwa.stage(data, 0U);

    EXPECT_FALSE(reader.consume(consumer_destination));
    EXPECT_EQ(consumer_destination.begin_called, 0);
    EXPECT_TRUE(consumer_destination.payload.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
    EXPECT_EQ(hwa.header_start_magic(), 0);
}

TEST_F(StagedUpdateReaderTest, RejectsWrongTargetUid)
{
    const auto data = payload();

    hwa.stage(data, common::dfu::dfu_stream_parser::START_COMMAND, common::dfu::dfu_stream_parser::FORMAT_VERSION, OPENDECK_TARGET_UID ^ 0x01U);

    EXPECT_FALSE(reader.consume(consumer_destination));
    EXPECT_EQ(consumer_destination.begin_called, 0);
    EXPECT_TRUE(consumer_destination.payload.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}
