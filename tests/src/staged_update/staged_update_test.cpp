/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "firmware/src/staged_update/hwa_test.h"
#include "firmware/src/staged_update/staged_update.h"

#include <zephyr/sys/crc.h>

#include <cstring>
#include <vector>

using namespace opendeck::staged_update;

namespace
{
    class StagedUpdateTest : public ::testing::Test
    {
        protected:
        HwaTest      hwa;
        StagedUpdate staged_update = StagedUpdate(hwa);

        static Metadata read_metadata(const HwaTest& hwa)
        {
            Metadata   metadata = {};
            const auto storage  = hwa.storage();
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

TEST_F(StagedUpdateTest, RejectsInvalidUploadSize)
{
    EXPECT_FALSE(staged_update.begin(0));
    EXPECT_FALSE(staged_update.begin(staged_update.capacity() + 1));
}

TEST_F(StagedUpdateTest, WritesPayloadAndCommitsMetadata)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update.begin(data.size()));
    ASSERT_TRUE(staged_update.write(std::span<const uint8_t>(data.data(), 4)));
    ASSERT_TRUE(staged_update.write(std::span<const uint8_t>(data.data() + 4, data.size() - 4)));
    ASSERT_TRUE(staged_update.finish());

    const auto metadata = read_metadata(hwa);
    EXPECT_EQ(metadata.magic, METADATA_MAGIC);
    EXPECT_EQ(metadata.format_version, METADATA_FORMAT_VERSION);
    EXPECT_EQ(metadata.target_uid, OPENDECK_TARGET_UID);
    EXPECT_EQ(metadata.size, data.size());
    EXPECT_EQ(metadata.crc32, crc32_ieee(data.data(), data.size()));
    EXPECT_EQ(staged_update.bytes_written(), data.size());

    const auto storage = hwa.storage();

    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(storage[METADATA_SIZE + i], data[i]);
    }

    for (size_t i = data.size(); i < HwaTest::WRITE_BLOCK_SIZE; i++)
    {
        EXPECT_EQ(storage[METADATA_SIZE + i], HwaTest::ERASED_BYTE);
    }
}

TEST_F(StagedUpdateTest, RejectsWritePastExpectedSize)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update.begin(2));
    EXPECT_FALSE(staged_update.write(std::span<const uint8_t>(data.data(), 3)));
    EXPECT_EQ(staged_update.bytes_written(), 0);

    const auto metadata = read_metadata(hwa);
    EXPECT_NE(metadata.magic, METADATA_MAGIC);
}

TEST_F(StagedUpdateTest, RejectsIncompleteUpload)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update.begin(data.size()));
    ASSERT_TRUE(staged_update.write(std::span<const uint8_t>(data.data(), data.size() - 1)));
    EXPECT_FALSE(staged_update.finish());
    EXPECT_EQ(staged_update.bytes_written(), 0);

    const auto metadata = read_metadata(hwa);
    EXPECT_NE(metadata.magic, METADATA_MAGIC);
}

TEST_F(StagedUpdateTest, AbortClearsMetadataSector)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update.begin(data.size()));
    ASSERT_TRUE(staged_update.write(data));
    staged_update.abort();

    ASSERT_FALSE(hwa.erase_calls().empty());
    EXPECT_EQ(hwa.erase_calls().back().offset, 0);
    EXPECT_EQ(hwa.erase_calls().back().size, HwaTest::SECTOR_SIZE);
    EXPECT_EQ(staged_update.bytes_written(), 0);
}

TEST_F(StagedUpdateTest, RejectsUnsupportedWriteBlockSize)
{
    hwa.set_write_block_size(0);

    EXPECT_FALSE(staged_update.begin(1));
}

TEST_F(StagedUpdateTest, AbortsWhenFlashWriteFails)
{
    const auto data = payload();

    ASSERT_TRUE(staged_update.begin(data.size()));
    hwa.set_write_result(false);

    EXPECT_FALSE(staged_update.write(std::span<const uint8_t>(data.data(), HwaTest::WRITE_BLOCK_SIZE)));
    EXPECT_EQ(staged_update.bytes_written(), 0);
}
