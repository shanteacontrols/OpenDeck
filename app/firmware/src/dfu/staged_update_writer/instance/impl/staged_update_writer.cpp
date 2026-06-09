/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/dfu/staged_update_writer/instance/impl/staged_update_writer.h"

#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>

using namespace opendeck::firmware::dfu::staged_update_writer;

namespace
{
    LOG_MODULE_REGISTER(staged_update_writer, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

StagedUpdateWriter::StagedUpdateWriter(opendeck::firmware::dfu::staged_update_writer::Hwa& hwa)
    : DfuWriter(hwa.flash_area())
    , _hwa(hwa)
{}

bool StagedUpdateWriter::begin(const opendeck::common::dfu::dfu_stream_parser::Header& header, const uint32_t expected_size)
{
    if (!DfuWriter::begin(header, expected_size))
    {
        return false;
    }

    if (!erase_header_sector())
    {
        DfuWriter::abort();
        return false;
    }

    return true;
}

bool StagedUpdateWriter::erase_header_sector()
{
    constexpr auto HEADER_SIZE = opendeck::common::dfu::staged_update::HEADER_STORAGE_SIZE;

    if (HEADER_SIZE == 0U)
    {
        LOG_ERR("Invalid staged DFU header storage size");
        return false;
    }

    uint32_t erased_size = 0;

    for (size_t index = 0; erased_size < HEADER_SIZE; index++)
    {
        const auto sector = _hwa.flash_area().sector(index);

        if (!sector)
        {
            LOG_ERR("Failed to read staged DFU header sector");
            return false;
        }

        if (!_hwa.flash_area().erase(sector->offset, sector->size))
        {
            LOG_ERR("Failed to erase staged DFU header sector");
            return false;
        }

        erased_size = sector->offset + sector->size;
    }

    return true;
}

bool StagedUpdateWriter::write_header(const opendeck::common::dfu::dfu_stream_parser::Header& header)
{
    constexpr auto HEADER_SIZE             = opendeck::common::dfu::staged_update::HEADER_STORAGE_SIZE;
    const size_t   native_write_block_size = _hwa.flash_area().write_block_size();

    if ((HEADER_SIZE == 0U) ||
        (native_write_block_size == 0U) ||
        ((HEADER_SIZE % native_write_block_size) != 0U))
    {
        LOG_ERR("Unsupported staged DFU flash write block size");
        return false;
    }

    std::array<uint8_t, HEADER_SIZE> data = {};

    std::fill(data.begin(), data.end(), opendeck::common::dfu::flash_area::ERASED_BYTE);
    std::copy(header.begin(), header.end(), data.begin());

    if (!_hwa.flash_area().write(0, std::span<const uint8_t>(data.data(), HEADER_SIZE)))
    {
        LOG_ERR("Failed to write staged DFU header");
        return false;
    }

    return true;
}

bool StagedUpdateWriter::commit(const opendeck::common::dfu::dfu_stream_parser::Header& header,
                                const uint32_t                                          expected_size)
{
    if (!write_header(header))
    {
        return false;
    }

    return DfuWriter::commit(header, expected_size);
}

void StagedUpdateWriter::cancel()
{
    erase_header_sector();
}

uint32_t StagedUpdateWriter::payload_offset() const
{
    return opendeck::common::dfu::staged_update::HEADER_STORAGE_SIZE;
}
