/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "bootloader/src/fw_selector/fw_selector.h"
#include "bootloader/src/updater/builder.h"
#include "bootloader/src/updater/common.h"
#include "app_validated_info.h"

#include <zephyr/sys/crc.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <span>
#include <utility>
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

    uint32_t calculate_payload_crc(const std::vector<uint8_t>& payload)
    {
        return crc32_ieee(payload.data(), payload.size());
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

    std::vector<uint8_t> make_validated_image(const std::vector<uint8_t>& payload)
    {
        std::vector<uint8_t> image = payload;

        while ((image.size() % sizeof(uint32_t)) != 0U)
        {
            image.push_back(0xFF);
        }

        append_u32(image, fw_selector::IMAGE_VALIDATION_MAGIC);
        append_u32(image, payload.size());
        append_u32(image, calculate_payload_crc(payload));

        return image;
    }

    void write_u32(std::vector<uint8_t>& data, size_t offset, uint32_t value)
    {
        constexpr uint32_t BYTE_MASK      = 0xFF;
        constexpr uint8_t  BITS_PER_OCTET = 8;

        ASSERT_LE(offset + sizeof(value), data.size());

        for (size_t i = 0; i < sizeof(value); i++)
        {
            data[offset + i] = (value >> (i * BITS_PER_OCTET)) & BYTE_MASK;
        }
    }

    void write_test_vector_table(std::vector<uint8_t>& image)
    {
        write_u32(image, 0, 0x20001000);
        write_u32(image, sizeof(uint32_t), 0x08000001);
    }

    void feed_stream(updater::Updater& updater, const std::vector<uint8_t>& stream)
    {
        for (const auto byte : stream)
        {
            updater.feed(byte);
        }
    }

    void assert_written_image(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& actual)
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

    std::vector<uint8_t> make_app_slot(const std::vector<uint8_t>& image)
    {
        std::vector<uint8_t> slot(OPENDECK_VALIDATED_APP_SIZE, 0xFF);

        EXPECT_LE(image.size(), slot.size());
        std::copy(image.begin(), image.end(), slot.begin());

        return slot;
    }

    fw_selector::AppInfo target_app_info()
    {
        return {
            .app = {
                .start = OPENDECK_VALIDATED_APP_START,
                .end   = OPENDECK_VALIDATED_APP_START + OPENDECK_VALIDATED_APP_SIZE,
            },
            .sram = {
                .start = OPENDECK_VALIDATED_SRAM_START,
                .end   = OPENDECK_VALIDATED_SRAM_START + OPENDECK_VALIDATED_SRAM_SIZE,
            },
            .vector_table_offset = OPENDECK_VALIDATED_ROM_START_OFFSET,
        };
    }

    class FwSelectorHwaTest : public fw_selector::Hwa
    {
        public:
        explicit FwSelectorHwaTest(std::vector<uint8_t> image)
            : image(std::move(image))
        {}

        bool is_hw_trigger_active() override
        {
            return false;
        }

        bool is_sw_trigger_active() override
        {
            return false;
        }

        bool read_app(const uint32_t offset, std::span<uint8_t> data) override
        {
            if (fail_read ||
                (offset > image.size()) ||
                (data.size() > (image.size() - offset)))
            {
                return false;
            }

            max_read_size = std::max(max_read_size, data.size());
            std::copy(image.begin() + offset, image.begin() + offset + data.size(), data.begin());
            return true;
        }

        fw_selector::AppInfo app_info() const override
        {
            return info;
        }

        void load(fw_selector::FwType) override
        {}

        std::vector<uint8_t> image;
        fw_selector::AppInfo info = {
            .sram = {
                .start = 0x20000000,
                .end   = 0x20010000,
            },
        };
        bool   fail_read     = false;
        size_t max_read_size = 0;
    };

}    // namespace

namespace opendeck::webusb
{
    void status(const char*)
    {
    }
}    // namespace opendeck::webusb

TEST(Bootloader, FwUpdate)
{
    const auto       validated_image = read_binary_file(OPENDECK_TEST_VALIDATED_PAYLOAD_FILE);
    const auto       update_stream   = read_binary_file(OPENDECK_TEST_DFU_FILE);
    const auto       page_count      = (validated_image.size() + BOOTLOADER_TEST_FLASH_PAGE_SIZE - 1U) / BOOTLOADER_TEST_FLASH_PAGE_SIZE;
    updater::HwaTest hwa(sizeof(uint32_t), BOOTLOADER_TEST_FLASH_PAGE_SIZE, page_count);
    updater::Updater updater(hwa);

    feed_stream(updater, update_stream);

    // once all data has been fed into updater, firmware update procedure should be complete
    ASSERT_TRUE(hwa.updated);

    assert_written_image(validated_image, hwa.written_bytes);

    FwSelectorHwaTest selector_hwa(make_app_slot(hwa.written_bytes));
    selector_hwa.info = target_app_info();

    fw_selector::FwSelector selector(selector_hwa);

    ASSERT_EQ(fw_selector::FwType::Application, selector.select().firmware);
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

        const auto image = make_validated_image(payload);

        feed_stream(updater, make_dfu_stream(image));

        ASSERT_TRUE(hwa.updated);
        assert_written_image(image, hwa.written_bytes);
    }
}

TEST(Bootloader, PadsPayloadShorterThanOneWriteBlock)
{
    const std::vector<uint8_t> payload = { 0x01, 0x02, 0x03 };
    updater::HwaTest           hwa(32, 64, 1);
    updater::Updater           updater(hwa);

    const auto image = make_validated_image(payload);

    feed_stream(updater, make_dfu_stream(image));

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(1, hwa.write_count);
    assert_written_image(image, hwa.written_bytes);
}

TEST(Bootloader, CommitsPayloadEndingMidPage)
{
    const std::vector<uint8_t> payload(40, 0x5A);
    updater::HwaTest           hwa(8, 128, 1);
    updater::Updater           updater(hwa);

    const auto image = make_validated_image(payload);

    feed_stream(updater, make_dfu_stream(image));

    const std::vector<size_t> expected_erased_pages = { 0 };

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(expected_erased_pages, hwa.erased_pages);
    ASSERT_EQ(7, hwa.write_count);
    assert_written_image(image, hwa.written_bytes);
}

TEST(Bootloader, WritesAcrossPageBoundary)
{
    std::vector<uint8_t> payload;

    for (size_t i = 0; i < 40; i++)
    {
        payload.push_back(static_cast<uint8_t>(i));
    }

    updater::HwaTest hwa(8, 16, 4);
    updater::Updater updater(hwa);

    const auto image = make_validated_image(payload);

    feed_stream(updater, make_dfu_stream(image));

    const std::vector<size_t> expected_erased_pages = { 0, 1, 2, 3 };

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(expected_erased_pages, hwa.erased_pages);
    assert_written_image(image, hwa.written_bytes);
}

TEST(Bootloader, WriteFailurePreventsApply)
{
    const std::vector<uint8_t> payload = { 0xAA, 0xBB, 0xCC, 0xDD };
    updater::HwaTest           hwa(4, 64, 1);
    updater::Updater           updater(hwa);

    hwa.fail_write = true;

    feed_stream(updater, make_dfu_stream(make_validated_image(payload)));

    ASSERT_FALSE(hwa.updated);
    ASSERT_TRUE(hwa.written_bytes.empty());
}

TEST(Bootloader, InvalidMetadataDoesNotEraseOrWrite)
{
    const std::vector<uint8_t> payload = { 0xAA, 0xBB, 0xCC, 0xDD };
    updater::HwaTest           hwa(4, 64, 1);
    updater::Updater           updater(hwa);

    const auto image = make_validated_image(payload);

    feed_stream(updater, make_dfu_stream(image, OPENDECK_TARGET_UID ^ 0x01));
    feed_stream(updater, make_dfu_stream(image, OPENDECK_TARGET_UID, updater::FORMAT_VERSION + 1));

    ASSERT_FALSE(hwa.updated);
    ASSERT_TRUE(hwa.erased_pages.empty());
    ASSERT_EQ(0, hwa.write_count);
}

TEST(Bootloader, RestartMarkerClearsStagedWriteState)
{
    const std::vector<uint8_t> abandoned_payload = { 0x01, 0x02, 0x03 };
    const std::vector<uint8_t> final_payload     = { 0x10, 0x11, 0x12, 0x13, 0x14 };

    const auto abandoned_image = make_validated_image(abandoned_payload);
    const auto final_image     = make_validated_image(final_payload);

    auto stream = make_dfu_stream(abandoned_image);
    stream.resize(sizeof(updater::START_COMMAND) + 12 + abandoned_image.size());

    const auto restart_stream = make_dfu_stream(final_image);
    stream.insert(stream.end(), restart_stream.begin(), restart_stream.end());

    updater::HwaTest hwa(8, 64, 1);
    updater::Updater updater(hwa);

    feed_stream(updater, stream);

    ASSERT_TRUE(hwa.updated);
    assert_written_image(final_image, hwa.written_bytes);
}

TEST(Bootloader, SelectorValidatesAppCrc)
{
    std::vector<uint8_t> payload = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90 };

    write_test_vector_table(payload);

    const auto image = make_validated_image(payload);

    FwSelectorHwaTest hwa(image);
    hwa.info.app = { .start = 0x08000000, .end = 0x08000000 + static_cast<uint32_t>(image.size()) };

    fw_selector::FwSelector selector(hwa);

    ASSERT_EQ(fw_selector::FwType::Application, selector.select().firmware);
}

TEST(Bootloader, SelectorScansValidationRecordsInBlocks)
{
    constexpr uint32_t slot_size = 1024;

    FwSelectorHwaTest hwa(std::vector<uint8_t>(slot_size, 0xFF));
    hwa.info.app = { .start = 0x08000000, .end = 0x08000000 + slot_size };

    write_test_vector_table(hwa.image);

    fw_selector::FwSelector selector(hwa);

    ASSERT_EQ(fw_selector::Trigger::InvalidApp, selector.select().trigger);
    ASSERT_GT(hwa.max_read_size, fw_selector::IMAGE_VALIDATION_RECORD_SIZE);
}

TEST(Bootloader, SelectorRejectsAppCrcMismatch)
{
    std::vector<uint8_t> payload = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90 };

    write_test_vector_table(payload);

    auto image = make_validated_image(payload);
    image.back() ^= 0x01U;

    FwSelectorHwaTest hwa(image);
    hwa.info.app = { .start = 0x08000000, .end = 0x08000000 + static_cast<uint32_t>(image.size()) };

    fw_selector::FwSelector selector(hwa);

    ASSERT_EQ(fw_selector::Trigger::InvalidApp, selector.select().trigger);
}

TEST(Bootloader, SelectorIgnoresStaleValidationRecord)
{
    std::vector<uint8_t> old_payload(96, 0xAA);
    std::vector<uint8_t> new_payload = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90 };

    write_test_vector_table(old_payload);
    write_test_vector_table(new_payload);

    auto old_image = make_validated_image(old_payload);
    auto new_image = make_validated_image(new_payload);

    std::copy(new_image.begin(), new_image.end(), old_image.begin());

    FwSelectorHwaTest hwa(old_image);
    hwa.info.app = { .start = 0x08000000, .end = 0x08000000 + static_cast<uint32_t>(old_image.size()) };

    fw_selector::FwSelector selector(hwa);

    ASSERT_EQ(fw_selector::FwType::Application, selector.select().firmware);
}

TEST(Bootloader, SelectorRejectsInvalidValidationRecordPlacement)
{
    const uint32_t slot_size = 128;

    FwSelectorHwaTest hwa(std::vector<uint8_t>(slot_size, 0));
    hwa.info.app = { .start = 0x08000000, .end = 0x08000000 + slot_size };

    write_test_vector_table(hwa.image);
    write_u32(hwa.image, 12, fw_selector::IMAGE_VALIDATION_MAGIC);
    write_u32(hwa.image, 16, 12);
    write_u32(hwa.image, 20, crc32_ieee(hwa.image.data(), 12));

    fw_selector::FwSelector selector(hwa);

    ASSERT_EQ(fw_selector::FwType::Application, selector.select().firmware);

    write_u32(hwa.image, 16, 8);
    write_u32(hwa.image, 20, 0);
    ASSERT_EQ(fw_selector::Trigger::InvalidApp, selector.select().trigger);

    write_u32(hwa.image, 12, 0);
    write_u32(hwa.image, slot_size - fw_selector::IMAGE_VALIDATION_RECORD_SIZE, fw_selector::IMAGE_VALIDATION_MAGIC);
    write_u32(hwa.image, slot_size - fw_selector::IMAGE_VALIDATION_RECORD_SIZE + sizeof(uint32_t), 128);
    write_u32(hwa.image, slot_size - fw_selector::IMAGE_VALIDATION_RECORD_SIZE + (sizeof(uint32_t) * 2U), 0);
    ASSERT_EQ(fw_selector::Trigger::InvalidApp, selector.select().trigger);
}
