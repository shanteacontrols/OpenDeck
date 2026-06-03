/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/dfu_stream.h"
#include "bootloader/src/io/indicators/builder/test/builder_test.h"
#include "bootloader/src/dfu/direct_update_writer/builder/builder.h"
#include "bootloader/src/signaling/signaling.h"
#include "bootloader/src/dfu/staged_update_reader/builder/test/builder_test.h"
#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace opendeck;

namespace
{
    std::vector<uint8_t> read_binary_file(const char* path)
    {
        std::ifstream file(path, std::ios::binary);

        EXPECT_TRUE(file.is_open()) << "Failed to open " << path;

        return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    void feed_stream(common::dfu::dfu_stream_parser::DfuStreamParser& parser, const std::vector<uint8_t>& stream)
    {
        parser.feed(stream);
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

    class SignalCapture
    {
        public:
        SignalCapture()
        {
            bootloader::signaling::subscribe<common::signaling::DfuStatusSignal>(
                [this](const common::signaling::DfuStatusSignal& signal)
                {
                    statuses.emplace_back(signal.message());
                });

            bootloader::signaling::subscribe<common::signaling::FirmwareUpdateStartedSignal>(
                [this](const common::signaling::FirmwareUpdateStartedSignal&)
                {
                    firmware_update_started_count++;
                });
        }

        std::vector<std::string> statuses;
        size_t                   firmware_update_started_count = 0;
    };

}    // namespace

TEST(Bootloader, FwUpdate)
{
    bootloader::signaling::clear_registry();

    const auto app_payload   = read_binary_file(OPENDECK_TEST_APP_PAYLOAD_FILE);
    const auto update_stream = read_binary_file(OPENDECK_TEST_DFU_FILE);
    const auto sector_count  = (app_payload.size() + BOOTLOADER_TEST_FLASH_SECTOR_SIZE - 1U) / BOOTLOADER_TEST_FLASH_SECTOR_SIZE;

    bootloader::dfu::direct_update_writer::HwaTest            hwa(sizeof(uint32_t), BOOTLOADER_TEST_FLASH_SECTOR_SIZE, sector_count);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    feed_stream(parser, update_stream);

    // once all data has been fed into the direct-update writer, firmware update procedure should be complete
    ASSERT_TRUE(hwa.updated);

    assert_written_image(app_payload, hwa.written_bytes);
}

TEST(Bootloader, SupportsDifferentFlashWriteBlockSizes)
{
    bootloader::signaling::clear_registry();

    const std::vector<uint8_t> payload = {
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34
    };

    for (const auto write_block_size : { 4U, 8U, 32U })
    {
        bootloader::dfu::direct_update_writer::HwaTest            hwa(write_block_size, 128, 1);
        bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
        common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

        feed_stream(parser, tests::dfu_stream_parser::make_stream(payload));

        ASSERT_TRUE(hwa.updated);
        assert_written_image(payload, hwa.written_bytes);
    }
}

TEST(Bootloader, PadsPayloadShorterThanOneWriteBlock)
{
    bootloader::signaling::clear_registry();

    const std::vector<uint8_t>                                payload = { 0x01, 0x02, 0x03 };
    bootloader::dfu::direct_update_writer::HwaTest            hwa(32, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload));

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(1, hwa.write_count);
    assert_written_image(payload, hwa.written_bytes);
}

TEST(Bootloader, CommitsPayloadEndingMidSector)
{
    bootloader::signaling::clear_registry();

    const std::vector<uint8_t>                                payload(40, 0x5A);
    bootloader::dfu::direct_update_writer::HwaTest            hwa(8, 128, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload));

    const std::vector<size_t> expected_erased_sectors = { 0 };

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(expected_erased_sectors, hwa.erased_sectors);
    ASSERT_EQ(1, hwa.write_count);
    assert_written_image(payload, hwa.written_bytes);
}

TEST(Bootloader, WritesAcrossSectorBoundary)
{
    bootloader::signaling::clear_registry();

    std::vector<uint8_t> payload;

    for (size_t i = 0; i < 40; i++)
    {
        payload.push_back(static_cast<uint8_t>(i));
    }

    bootloader::dfu::direct_update_writer::HwaTest            hwa(8, 16, 4);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload));

    const std::vector<size_t> expected_erased_sectors = { 0, 1, 2 };

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(expected_erased_sectors, hwa.erased_sectors);
    assert_written_image(payload, hwa.written_bytes);
}

TEST(Bootloader, WriteFailurePreventsApply)
{
    bootloader::signaling::clear_registry();

    const std::vector<uint8_t>                                payload = { 0xAA, 0xBB, 0xCC, 0xDD };
    bootloader::dfu::direct_update_writer::HwaTest            hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    hwa.fail_write = true;

    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload));

    ASSERT_FALSE(hwa.updated);
    ASSERT_TRUE(hwa.written_bytes.empty());
}

TEST(Bootloader, PayloadTooLargeDoesNotEraseOrWrite)
{
    bootloader::signaling::clear_registry();

    const std::vector<uint8_t>                                payload(65, 0xAA);
    bootloader::dfu::direct_update_writer::HwaTest            hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload));

    ASSERT_FALSE(hwa.updated);
    ASSERT_TRUE(hwa.erased_sectors.empty());
    ASSERT_EQ(0, hwa.write_count);
}

TEST(Bootloader, InvalidDfuHeaderDoesNotEraseOrWrite)
{
    bootloader::signaling::clear_registry();

    const std::vector<uint8_t>                                payload = { 0xAA, 0xBB, 0xCC, 0xDD };
    bootloader::dfu::direct_update_writer::HwaTest            hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload, OPENDECK_TARGET_UID ^ 0x01));
    parser.reset();
    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload, OPENDECK_TARGET_UID, common::dfu::dfu_stream_parser::FORMAT_VERSION + 1));

    ASSERT_FALSE(hwa.updated);
    ASSERT_TRUE(hwa.erased_sectors.empty());
    ASSERT_EQ(0, hwa.write_count);
}

TEST(Bootloader, DirectUpdateWriterPublishesUpdateLifecycleSignals)
{
    bootloader::signaling::clear_registry();

    SignalCapture                                             capture;
    const std::vector<uint8_t>                                payload = { 0xAA, 0xBB, 0xCC, 0xDD };
    bootloader::dfu::direct_update_writer::HwaTest            hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(hwa, hwa);
    common::dfu::dfu_stream_parser::DfuStreamParser           parser(writer);

    feed_stream(parser, tests::dfu_stream_parser::make_stream(payload));

    ASSERT_TRUE(hwa.updated);
    ASSERT_EQ(1, capture.firmware_update_started_count);
    ASSERT_NE(std::find(capture.statuses.begin(), capture.statuses.end(), "DFU header accepted"), capture.statuses.end());
    ASSERT_NE(std::find(capture.statuses.begin(), capture.statuses.end(), "Firmware update started"), capture.statuses.end());
    ASSERT_NE(std::find(capture.statuses.begin(), capture.statuses.end(), "Firmware update complete, rebooting"), capture.statuses.end());
}

TEST(Bootloader, StagedUpdateReaderPublishesUpdateStartedSignal)
{
    bootloader::signaling::clear_registry();

    SignalCapture                                             capture;
    const std::vector<uint8_t>                                payload = { 0x10, 0x11, 0x12, 0x13 };
    bootloader::dfu::staged_update_reader::Builder            staged_update_reader;
    bootloader::dfu::direct_update_writer::HwaTest            writer_hwa(4, 64, 1);
    bootloader::dfu::direct_update_writer::DirectUpdateWriter writer(writer_hwa, writer_hwa);

    staged_update_reader._hwa.stage(payload);

    ASSERT_TRUE(staged_update_reader.instance().consume(writer));
    ASSERT_TRUE(writer_hwa.updated);
    ASSERT_EQ(1, capture.firmware_update_started_count);
}

TEST(Bootloader, IndicatorsBlinkWhenFirmwareUpdateStarts)
{
    bootloader::signaling::clear_registry();

    bootloader::io::indicators::Builder indicators;

    ASSERT_TRUE(indicators.instance().init());
    ASSERT_TRUE(indicators._hwa._init_called);
    ASSERT_TRUE(indicators._hwa._on_called);
    ASSERT_FALSE(indicators._hwa._off_called);

    bootloader::signaling::publish(common::signaling::FirmwareUpdateStartedSignal{});

    ASSERT_TRUE(indicators._hwa._off_called);
}
