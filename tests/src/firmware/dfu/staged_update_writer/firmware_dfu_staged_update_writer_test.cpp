/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "common/src/dfu/dfu_stream_parser/shared/common.h"
#include "common/src/dfu/flash_area/hwa/test/hwa_test.h"
#include "common/src/dfu/staged_update/shared/deps.h"
#include "firmware/src/dfu/staged_update_writer/hwa/test/hwa_test.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/staged_update_writer.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

using namespace opendeck;
using namespace opendeck::firmware::dfu::staged_update_writer;

namespace
{
    using FlashAreaHwaTest = common::dfu::flash_area::HwaTest;

    class StagedUpdateWriterTest : public ::testing::Test
    {
        protected:
        HwaTest            hwa;
        StagedUpdateWriter staged_update_writer = StagedUpdateWriter(hwa);

        static void write_word(common::dfu::dfu_stream_parser::Header& header, size_t word_index, uint32_t value)
        {
            constexpr uint32_t BYTE_MASK      = 0xFF;
            constexpr uint8_t  BITS_PER_OCTET = 8;
            const size_t       offset         = word_index * sizeof(value);

            for (size_t i = 0; i < sizeof(value); i++)
            {
                header[offset + i] = (value >> (i * BITS_PER_OCTET)) & BYTE_MASK;
            }
        }

        static common::dfu::dfu_stream_parser::Header header(uint32_t payload_size)
        {
            common::dfu::dfu_stream_parser::Header header = {};

            write_word(header, 0, common::dfu::dfu_stream_parser::START_COMMAND);
            write_word(header, 1, common::dfu::dfu_stream_parser::FORMAT_VERSION);
            write_word(header, 2, OPENDECK_TARGET_UID);
            write_word(header, 3, payload_size);

            return header;
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

TEST_F(StagedUpdateWriterTest, ReportsBackendSupport)
{
    EXPECT_TRUE(staged_update_writer.supported());
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
        EXPECT_EQ(storage[common::dfu::staged_update::StagedUpdate::header_storage_size() + i], data[i]);
    }

    for (size_t i = data.size(); i < FlashAreaHwaTest::SECTOR_SIZE; i++)
    {
        EXPECT_EQ(storage[common::dfu::staged_update::StagedUpdate::header_storage_size() + i], common::dfu::flash_area::ERASED_BYTE);
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

    ASSERT_EQ(hwa.erase_calls().size(), common::dfu::staged_update::StagedUpdate::header_storage_size() / FlashAreaHwaTest::SECTOR_SIZE);

    for (size_t i = 0; i < hwa.erase_calls().size(); i++)
    {
        EXPECT_EQ(hwa.erase_calls()[i].offset, i * FlashAreaHwaTest::SECTOR_SIZE);
        EXPECT_EQ(hwa.erase_calls()[i].size, FlashAreaHwaTest::SECTOR_SIZE);
    }
}

TEST_F(StagedUpdateWriterTest, SelectsPayloadOffsetAlignedWriteBlockSize)
{
    const auto data       = payload();
    const auto dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));

    EXPECT_EQ(staged_update_writer.write_block_size(), FlashAreaHwaTest::SECTOR_SIZE);
}

TEST_F(StagedUpdateWriterTest, ErasesPayloadSectorsAsNeeded)
{
    const std::vector<uint8_t> data(FlashAreaHwaTest::SECTOR_SIZE, 0x42);
    const auto                 dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));
    ASSERT_TRUE(staged_update_writer.write(data));
    ASSERT_TRUE(staged_update_writer.finish());

    constexpr size_t HEADER_SECTOR_COUNT = common::dfu::staged_update::StagedUpdate::header_storage_size() / FlashAreaHwaTest::SECTOR_SIZE;

    ASSERT_EQ(hwa.erase_calls().size(), HEADER_SECTOR_COUNT + 1U);

    for (size_t i = 0; i < HEADER_SECTOR_COUNT; i++)
    {
        EXPECT_EQ(hwa.erase_calls()[i].offset, i * FlashAreaHwaTest::SECTOR_SIZE);
        EXPECT_EQ(hwa.erase_calls()[i].size, FlashAreaHwaTest::SECTOR_SIZE);
    }

    EXPECT_EQ(hwa.erase_calls()[HEADER_SECTOR_COUNT].offset, common::dfu::staged_update::StagedUpdate::header_storage_size());
    EXPECT_EQ(hwa.erase_calls()[HEADER_SECTOR_COUNT].size, FlashAreaHwaTest::SECTOR_SIZE);
}

TEST_F(StagedUpdateWriterTest, AbortClearsHeaderSector)
{
    const auto data       = payload();
    const auto dfu_header = header(data.size());

    ASSERT_TRUE(staged_update_writer.begin(dfu_header, data.size()));
    ASSERT_TRUE(staged_update_writer.write(data));
    staged_update_writer.abort();

    constexpr size_t HEADER_SECTOR_COUNT = common::dfu::staged_update::StagedUpdate::header_storage_size() / FlashAreaHwaTest::SECTOR_SIZE;

    ASSERT_GE(hwa.erase_calls().size(), HEADER_SECTOR_COUNT);

    const size_t first_abort_erase = hwa.erase_calls().size() - HEADER_SECTOR_COUNT;

    for (size_t i = 0; i < HEADER_SECTOR_COUNT; i++)
    {
        EXPECT_EQ(hwa.erase_calls()[first_abort_erase + i].offset, i * FlashAreaHwaTest::SECTOR_SIZE);
        EXPECT_EQ(hwa.erase_calls()[first_abort_erase + i].size, FlashAreaHwaTest::SECTOR_SIZE);
    }
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

    EXPECT_TRUE(staged_update_writer.write(data));
    EXPECT_FALSE(staged_update_writer.finish());
}
