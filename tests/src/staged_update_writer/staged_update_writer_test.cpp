/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "common/src/flash_area/hwa/test/hwa_test.h"
#include "common/src/staged_update/shared/common.h"
#include "firmware/src/staged_update_writer/hwa/test/hwa_test.h"
#include "firmware/src/staged_update_writer/instance/impl/staged_update_writer.h"

#include <zephyr/sys/crc.h>

#include <cstring>
#include <vector>

using namespace opendeck::staged_update_writer;

namespace
{
    using FlashAreaHwaTest = opendeck::flash_area::HwaTest;

    class StagedUpdateWriterTest : public ::testing::Test
    {
        protected:
        HwaTest            hwa;
        StagedUpdateWriter staged_update_writer = StagedUpdateWriter(hwa);

        static opendeck::staged_update::Metadata read_metadata(const HwaTest& hwa)
        {
            opendeck::staged_update::Metadata metadata = {};
            const auto                        storage  = hwa.storage();
            std::memcpy(&metadata, storage.data(), sizeof(metadata));
            return metadata;
        }

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
    };
}    // namespace

TEST_F(StagedUpdateWriterTest, RejectsInvalidUploadSize)
{
    EXPECT_FALSE(staged_update_writer.begin(0));
    EXPECT_FALSE(staged_update_writer.begin(staged_update_writer.capacity() + 1));
}

TEST_F(StagedUpdateWriterTest, WritesPayloadAndCommitsMetadata)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update_writer.begin(data.size()));
    ASSERT_TRUE(staged_update_writer.write(std::span<const uint8_t>(data.data(), 4)));
    ASSERT_TRUE(staged_update_writer.write(std::span<const uint8_t>(data.data() + 4, data.size() - 4)));
    ASSERT_TRUE(staged_update_writer.finish());

    const auto metadata = read_metadata(hwa);
    EXPECT_EQ(metadata.magic, opendeck::staged_update::METADATA_MAGIC);
    EXPECT_EQ(metadata.format_version, opendeck::staged_update::METADATA_FORMAT_VERSION);
    EXPECT_EQ(metadata.target_uid, OPENDECK_TARGET_UID);
    EXPECT_EQ(metadata.size, data.size());
    EXPECT_EQ(metadata.crc32, crc32_ieee(data.data(), data.size()));
    EXPECT_EQ(staged_update_writer.bytes_written(), data.size());

    const auto storage = hwa.storage();

    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(storage[opendeck::staged_update::METADATA_SIZE + i], data[i]);
    }

    for (size_t i = data.size(); i < FlashAreaHwaTest::WRITE_BLOCK_SIZE; i++)
    {
        EXPECT_EQ(storage[opendeck::staged_update::METADATA_SIZE + i], FlashAreaHwaTest::ERASED_BYTE);
    }
}

TEST_F(StagedUpdateWriterTest, RejectsWritePastExpectedSize)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update_writer.begin(2));
    EXPECT_FALSE(staged_update_writer.write(std::span<const uint8_t>(data.data(), 3)));
    EXPECT_EQ(staged_update_writer.bytes_written(), 0);

    const auto metadata = read_metadata(hwa);
    EXPECT_NE(metadata.magic, opendeck::staged_update::METADATA_MAGIC);
}

TEST_F(StagedUpdateWriterTest, RejectsIncompleteUpload)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update_writer.begin(data.size()));
    ASSERT_TRUE(staged_update_writer.write(std::span<const uint8_t>(data.data(), data.size() - 1)));
    EXPECT_FALSE(staged_update_writer.finish());
    EXPECT_EQ(staged_update_writer.bytes_written(), 0);

    const auto metadata = read_metadata(hwa);
    EXPECT_NE(metadata.magic, opendeck::staged_update::METADATA_MAGIC);
}

TEST_F(StagedUpdateWriterTest, AbortClearsMetadataSector)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update_writer.begin(data.size()));
    ASSERT_TRUE(staged_update_writer.write(data));
    staged_update_writer.abort();

    ASSERT_FALSE(hwa.erase_calls().empty());
    EXPECT_EQ(hwa.erase_calls().back().offset, 0);
    EXPECT_EQ(hwa.erase_calls().back().size, FlashAreaHwaTest::SECTOR_SIZE);
    EXPECT_EQ(staged_update_writer.bytes_written(), 0);
}

TEST_F(StagedUpdateWriterTest, RejectsUnsupportedWriteBlockSize)
{
    hwa.set_write_block_size(0);

    EXPECT_FALSE(staged_update_writer.begin(1));
}

TEST_F(StagedUpdateWriterTest, AbortsWhenFlashWriteFails)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update_writer.begin(data.size()));
    hwa.set_write_result(false);

    EXPECT_FALSE(staged_update_writer.write(std::span<const uint8_t>(data.data(), FlashAreaHwaTest::WRITE_BLOCK_SIZE)));
    EXPECT_EQ(staged_update_writer.bytes_written(), 0);
}
