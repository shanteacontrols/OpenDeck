/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "common/src/dfu/dfu_stream/shared/common.h"
#include "common/src/dfu/flash_area/hwa/test/hwa_test.h"
#include "firmware/src/dfu/staged_update_writer/hwa/test/hwa_test.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/staged_update_writer.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

using namespace opendeck::firmware::dfu::staged_update_writer;

namespace
{
    using FlashAreaHwaTest = opendeck::common::dfu::flash_area::HwaTest;

    class StagedUpdateWriterTest : public ::testing::Test
    {
        protected:
        HwaTest            hwa;
        StagedUpdateWriter staged_update_writer = StagedUpdateWriter(hwa);

        static void write_word(opendeck::common::dfu::dfu_stream::Header& header, size_t word_index, uint32_t value)
        {
            constexpr uint32_t BYTE_MASK      = 0xFF;
            constexpr uint8_t  BITS_PER_OCTET = 8;
            const size_t       offset         = word_index * sizeof(value);

            for (size_t i = 0; i < sizeof(value); i++)
            {
                header[offset + i] = (value >> (i * BITS_PER_OCTET)) & BYTE_MASK;
            }
        }

        static opendeck::common::dfu::dfu_stream::Header header(uint32_t payload_size)
        {
            opendeck::common::dfu::dfu_stream::Header header = {};

            write_word(header, 0, opendeck::common::dfu::dfu_stream::START_COMMAND);
            write_word(header, 1, opendeck::common::dfu::dfu_stream::FORMAT_VERSION);
            write_word(header, 2, OPENDECK_TARGET_UID);
            write_word(header, 3, payload_size);

            return header;
        }

        static constexpr size_t header_storage_size()
        {
            return ((opendeck::common::dfu::dfu_stream::HEADER_SIZE + FlashAreaHwaTest::WRITE_BLOCK_SIZE - 1U) /
                    FlashAreaHwaTest::WRITE_BLOCK_SIZE) *
                   FlashAreaHwaTest::WRITE_BLOCK_SIZE;
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
    EXPECT_FALSE(staged_update_writer.begin(header(0), 0));
    EXPECT_FALSE(staged_update_writer.begin(header(hwa.size()), hwa.size()));
}

TEST_F(StagedUpdateWriterTest, WritesHeaderAndPayload)
{
    const auto data = payload();

    const auto dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));
    ASSERT_TRUE(staged_update_writer.write(std::span<const uint8_t>(data.data(), 4)));
    ASSERT_TRUE(staged_update_writer.write(std::span<const uint8_t>(data.data() + 4, data.size() - 4)));
    ASSERT_TRUE(staged_update_writer.finish());

    const auto storage = hwa.storage();
    EXPECT_TRUE(std::equal(dfu_header.begin(), dfu_header.end(), storage.begin()));

    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(storage[header_storage_size() + i], data[i]);
    }

    for (size_t i = data.size(); i < FlashAreaHwaTest::WRITE_BLOCK_SIZE; i++)
    {
        EXPECT_EQ(storage[header_storage_size() + i], FlashAreaHwaTest::ERASED_BYTE);
    }
}

TEST_F(StagedUpdateWriterTest, WritesHeaderOnlyAfterPayloadFinish)
{
    const auto data       = payload();
    const auto dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));

    const auto storage_before_finish = hwa.storage();
    EXPECT_FALSE(std::equal(dfu_header.begin(), dfu_header.end(), storage_before_finish.begin()));

    ASSERT_TRUE(staged_update_writer.write(data));

    const auto storage_after_write = hwa.storage();
    EXPECT_FALSE(std::equal(dfu_header.begin(), dfu_header.end(), storage_after_write.begin()));

    ASSERT_TRUE(staged_update_writer.finish());

    const auto storage_after_finish = hwa.storage();
    EXPECT_TRUE(std::equal(dfu_header.begin(), dfu_header.end(), storage_after_finish.begin()));
}

TEST_F(StagedUpdateWriterTest, BeginOnlyInvalidatesHeaderSector)
{
    const auto data       = payload();
    const auto dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));

    ASSERT_EQ(hwa.erase_calls().size(), 1U);
    EXPECT_EQ(hwa.erase_calls().front().offset, 0);
    EXPECT_EQ(hwa.erase_calls().front().size, FlashAreaHwaTest::SECTOR_SIZE);
}

TEST_F(StagedUpdateWriterTest, ErasesPayloadSectorsAsNeeded)
{
    const std::vector<uint8_t> data(FlashAreaHwaTest::SECTOR_SIZE, 0x42);
    const auto                 dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));
    ASSERT_TRUE(staged_update_writer.write(data));
    ASSERT_TRUE(staged_update_writer.finish());

    ASSERT_EQ(hwa.erase_calls().size(), 2U);
    EXPECT_EQ(hwa.erase_calls()[0].offset, 0);
    EXPECT_EQ(hwa.erase_calls()[0].size, FlashAreaHwaTest::SECTOR_SIZE);
    EXPECT_EQ(hwa.erase_calls()[1].offset, FlashAreaHwaTest::SECTOR_SIZE);
    EXPECT_EQ(hwa.erase_calls()[1].size, FlashAreaHwaTest::SECTOR_SIZE);
}

TEST_F(StagedUpdateWriterTest, AbortClearsHeaderSector)
{
    const auto data       = payload();
    const auto dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));
    ASSERT_TRUE(staged_update_writer.write(data));
    staged_update_writer.abort();

    ASSERT_FALSE(hwa.erase_calls().empty());
    EXPECT_EQ(hwa.erase_calls().back().offset, 0);
    EXPECT_EQ(hwa.erase_calls().back().size, FlashAreaHwaTest::SECTOR_SIZE);
}

TEST_F(StagedUpdateWriterTest, RejectsUnsupportedWriteBlockSize)
{
    hwa.set_write_block_size(0);

    EXPECT_FALSE(staged_update_writer.begin(header(1), 1));
}

TEST_F(StagedUpdateWriterTest, AbortsWhenFlashWriteFails)
{
    const auto data       = payload();
    const auto dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));
    hwa.set_write_result(false);

    EXPECT_FALSE(staged_update_writer.write(std::span<const uint8_t>(data.data(), FlashAreaHwaTest::WRITE_BLOCK_SIZE)));
}
