/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "bootloader/src/staged_update_reader/hwa/test/hwa_test.h"
#include "bootloader/src/staged_update_reader/instance/impl/staged_update_reader.h"

#include <optional>
#include <vector>

using namespace opendeck;

namespace
{
    class ConsumerTest : public staged_update_reader::Consumer
    {
        public:
        void reset() override
        {
            reset_count++;
            received.clear();
        }

        staged_update_reader::StreamStatus feed(uint8_t byte) override
        {
            received.push_back(byte);

            if ((invalid_after != 0) && (received.size() >= invalid_after))
            {
                return staged_update_reader::StreamStatus::Invalid;
            }

            if ((complete_after != 0) && (received.size() >= complete_after))
            {
                return staged_update_reader::StreamStatus::Complete;
            }

            return staged_update_reader::StreamStatus::Incomplete;
        }

        size_t               reset_count    = 0;
        size_t               complete_after = 0;
        size_t               invalid_after  = 0;
        std::vector<uint8_t> received       = {};
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

        staged_update_reader::HwaTest            hwa;
        staged_update_reader::StagedUpdateReader reader = staged_update_reader::StagedUpdateReader(hwa);
        ConsumerTest                             consumer;
    };
}    // namespace

TEST_F(StagedUpdateReaderTest, IgnoresMissingPendingUpdate)
{
    EXPECT_FALSE(reader.consume(consumer));
    EXPECT_TRUE(hwa.init_called());
    EXPECT_EQ(consumer.reset_count, 0);
    EXPECT_TRUE(consumer.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}

TEST_F(StagedUpdateReaderTest, IgnoresUnavailableStorage)
{
    hwa.set_init_result(false);

    EXPECT_FALSE(reader.consume(consumer));
    EXPECT_TRUE(hwa.init_called());
    EXPECT_EQ(consumer.reset_count, 0);
    EXPECT_TRUE(consumer.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
}

TEST_F(StagedUpdateReaderTest, StreamsValidPendingUpdateAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);
    consumer.complete_after = data.size();

    EXPECT_TRUE(reader.consume(consumer));
    EXPECT_EQ(consumer.reset_count, 1);
    EXPECT_EQ(consumer.received, data);
    EXPECT_EQ(hwa.update_start_calls(), 1);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_EQ(hwa.metadata_magic(), 0);
}

TEST_F(StagedUpdateReaderTest, RejectsCrcMismatchAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data, 0x12345678U);

    EXPECT_FALSE(reader.consume(consumer));
    EXPECT_EQ(consumer.reset_count, 0);
    EXPECT_TRUE(consumer.received.empty());
    EXPECT_EQ(hwa.update_start_calls(), 0);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_EQ(hwa.metadata_magic(), 0);
}

TEST_F(StagedUpdateReaderTest, RejectsInvalidConsumerStatusAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);
    consumer.invalid_after = 3;

    EXPECT_FALSE(reader.consume(consumer));
    EXPECT_EQ(consumer.reset_count, 1);
    EXPECT_EQ(consumer.received.size(), 3);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_EQ(hwa.metadata_magic(), 0);
}

TEST_F(StagedUpdateReaderTest, RejectsIncompleteConsumerAndClearsMarker)
{
    const auto data = payload();

    hwa.stage(data);

    EXPECT_FALSE(reader.consume(consumer));
    EXPECT_EQ(consumer.reset_count, 1);
    EXPECT_EQ(consumer.received, data);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
    EXPECT_EQ(hwa.metadata_magic(), 0);
}

TEST_F(StagedUpdateReaderTest, RejectsInvalidMetadataWithoutClearingAgain)
{
    const auto data = payload();

    hwa.stage(data, std::nullopt, 0U);

    EXPECT_FALSE(reader.consume(consumer));
    EXPECT_EQ(consumer.reset_count, 0);
    EXPECT_TRUE(consumer.received.empty());
    EXPECT_EQ(hwa.clear_pending_calls(), 0);
    EXPECT_EQ(hwa.metadata_magic(), 0);
}

TEST_F(StagedUpdateReaderTest, IgnoresMetadataTargetUid)
{
    const auto data = payload();

    hwa.stage(
        data,
        std::nullopt,
        staged_update::METADATA_MAGIC,
        staged_update::METADATA_FORMAT_VERSION,
        OPENDECK_TARGET_UID ^ 0x01U);
    consumer.complete_after = data.size();

    EXPECT_TRUE(reader.consume(consumer));
    EXPECT_EQ(consumer.reset_count, 1);
    EXPECT_EQ(consumer.received, data);
    EXPECT_EQ(hwa.clear_pending_calls(), 1);
}
