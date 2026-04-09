/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "updater/builder.h"
#include "updater/common.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

namespace
{
    std::vector<uint8_t> read_binary_file(const char* path)
    {
        std::ifstream file(path, std::ios::binary);

        EXPECT_TRUE(file.is_open()) << "Failed to open " << path;

        return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    void append_u32(std::vector<uint8_t>& data, uint32_t value)
    {
        constexpr uint32_t BYTE_MASK      = 0xFF;
        constexpr uint8_t  BITS_PER_OCTET = 8;

        for (size_t i = 0; i < sizeof(value); i++)
        {
            data.push_back((value >> (i * BITS_PER_OCTET)) & BYTE_MASK);
        }
    }

    std::vector<uint8_t> make_dfu_stream(const std::vector<uint8_t>& payload,
                                         uint32_t                    target_uid     = OPENDECK_TARGET_UID,
                                         uint32_t                    format_version = updater::FORMAT_VERSION)
    {
        std::vector<uint8_t> stream;

        append_u32(stream, updater::START_COMMAND);
        append_u32(stream, format_version);
        append_u32(stream, target_uid);
        append_u32(stream, payload.size());
        stream.insert(stream.end(), payload.begin(), payload.end());
        append_u32(stream, updater::END_COMMAND);

        return stream;
    }

    void feed_stream(updater::Updater& updater, const std::vector<uint8_t>& stream)
    {
        for (const auto byte : stream)
        {
            updater.feed(byte);
        }
    }

    void assert_written_payload(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& actual)
    {
        ASSERT_GE(actual.size(), expected.size());

        for (size_t i = 0; i < expected.size(); i++)
        {
            ASSERT_EQ(expected.at(i), actual.at(i)) << "Difference on byte " << i;
        }

        for (size_t i = expected.size(); i < actual.size(); i++)
        {
            ASSERT_EQ(0xFF, actual.at(i)) << "Unexpected padding on byte " << i;
        }
    }

}    // namespace

namespace webusb
{
    void status(const char*)
    {
    }
}    // namespace webusb

TEST(Bootloader, FwUpdate)
{
    updater::Builder builder_updater;

    const auto binary_vector = read_binary_file(OPENDECK_TEST_PAYLOAD_FILE);
    const auto update_stream = read_binary_file(OPENDECK_TEST_DFU_FILE);

    feed_stream(builder_updater.instance(), update_stream);

    // once all data has been fed into updater, firmware update procedure should be complete
    ASSERT_TRUE(builder_updater._hwa.updated);

    assert_written_payload(binary_vector, builder_updater._hwa.written_bytes);
}

TEST(Bootloader, SupportsDifferentFlashWriteBlockSizes)
{
    const std::vector<uint8_t> payload = {
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34
    };

    for (const auto write_block_size : { 4U, 8U, 32U })
    {
        updater::HwaTest hwa(write_block_size, 128, 1);
        updater::Updater updater(hwa);

        feed_stream(updater, make_dfu_stream(payload));

        ASSERT_TRUE(hwa.updated);
        assert_written_payload(payload, hwa.written_bytes);
    }
}

TEST(Bootloader, PadsPayloadShorterThanOneWriteBlock)
{
    const std::vector<uint8_t> payload = { 0x01, 0x02, 0x03 };
    updater::HwaTest           hwa(32, 64, 1);
    updater::Updater           updater(hwa);

    feed_stream(updater, make_dfu_stream(payload));

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(1, hwa.write_count);
    assert_written_payload(payload, hwa.written_bytes);
}

TEST(Bootloader, CommitsPayloadEndingMidPage)
{
    const std::vector<uint8_t> payload(40, 0x5A);
    updater::HwaTest           hwa(8, 128, 1);
    updater::Updater           updater(hwa);

    feed_stream(updater, make_dfu_stream(payload));

    const std::vector<size_t> expected_erased_pages = { 0 };

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(expected_erased_pages, hwa.erased_pages);
    ASSERT_EQ(5, hwa.write_count);
    assert_written_payload(payload, hwa.written_bytes);
}

TEST(Bootloader, WritesAcrossPageBoundary)
{
    std::vector<uint8_t> payload;

    for (size_t i = 0; i < 40; i++)
    {
        payload.push_back(static_cast<uint8_t>(i));
    }

    updater::HwaTest hwa(8, 16, 3);
    updater::Updater updater(hwa);

    feed_stream(updater, make_dfu_stream(payload));

    const std::vector<size_t> expected_erased_pages = { 0, 1, 2 };

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(expected_erased_pages, hwa.erased_pages);
    assert_written_payload(payload, hwa.written_bytes);
}

TEST(Bootloader, WriteFailurePreventsApply)
{
    const std::vector<uint8_t> payload = { 0xAA, 0xBB, 0xCC, 0xDD };
    updater::HwaTest           hwa(4, 64, 1);
    updater::Updater           updater(hwa);

    hwa.fail_write = true;

    feed_stream(updater, make_dfu_stream(payload));

    ASSERT_FALSE(hwa.updated);
    ASSERT_TRUE(hwa.written_bytes.empty());
}

TEST(Bootloader, InvalidMetadataDoesNotEraseOrWrite)
{
    const std::vector<uint8_t> payload = { 0xAA, 0xBB, 0xCC, 0xDD };
    updater::HwaTest           hwa(4, 64, 1);
    updater::Updater           updater(hwa);

    feed_stream(updater, make_dfu_stream(payload, OPENDECK_TARGET_UID ^ 0x01));
    feed_stream(updater, make_dfu_stream(payload, OPENDECK_TARGET_UID, updater::FORMAT_VERSION + 1));

    ASSERT_FALSE(hwa.updated);
    ASSERT_TRUE(hwa.erased_pages.empty());
    ASSERT_EQ(0, hwa.write_count);
}

TEST(Bootloader, RestartMarkerClearsStagedWriteState)
{
    const std::vector<uint8_t> abandoned_payload = { 0x01, 0x02, 0x03 };
    const std::vector<uint8_t> final_payload     = { 0x10, 0x11, 0x12, 0x13, 0x14 };

    auto stream = make_dfu_stream(abandoned_payload);
    stream.resize(sizeof(updater::START_COMMAND) + 12 + abandoned_payload.size());

    const auto restart_stream = make_dfu_stream(final_payload);
    stream.insert(stream.end(), restart_stream.begin(), restart_stream.end());

    updater::HwaTest hwa(8, 64, 1);
    updater::Updater updater(hwa);

    feed_stream(updater, stream);

    ASSERT_TRUE(hwa.updated);
    assert_written_payload(final_payload, hwa.written_bytes);
}
