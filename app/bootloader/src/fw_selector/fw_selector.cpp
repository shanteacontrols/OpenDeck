/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fw_selector.h"

#include <zephyr/sys/crc.h>

#include <algorithm>
#include <array>

namespace
{
    constexpr uint32_t FLASH_SCAN_BLOCK_SIZE            = 256;
    constexpr uint32_t VALIDATION_SP_ALIGNMENT_BYTES    = 4;
    constexpr uint32_t THUMB_BIT_MASK                   = 0x01;
    constexpr uint32_t VALIDATION_RECORD_ALIGNMENT_SIZE = sizeof(uint32_t);
    constexpr uint8_t  BYTE_1_SHIFT_BITS                = 8;
    constexpr uint8_t  BYTE_2_SHIFT_BITS                = 16;
    constexpr uint8_t  BYTE_3_SHIFT_BITS                = 24;

    uint32_t read_le32(const uint8_t* data)
    {
        return static_cast<uint32_t>(data[0]) |
               (static_cast<uint32_t>(data[1]) << BYTE_1_SHIFT_BITS) |
               (static_cast<uint32_t>(data[2]) << BYTE_2_SHIFT_BITS) |
               (static_cast<uint32_t>(data[3]) << BYTE_3_SHIFT_BITS);
    }

    bool is_aligned(const uint32_t value, const uint32_t alignment)
    {
        return (value % alignment) == 0U;
    }

    bool is_in_range(const uint32_t value, const uint32_t start, const uint32_t end)
    {
        return (value >= start) && (value < end);
    }
}    // namespace

fw_selector::Selection fw_selector::FwSelector::select()
{
    Selection selection = {};

    if (_hwa.is_hw_trigger_active())
    {
        selection.firmware = FwType::Bootloader;
        selection.trigger  = Trigger::Hardware;

        return selection;
    }

    if (_hwa.is_sw_trigger_active())
    {
        selection.firmware = FwType::Bootloader;
        selection.trigger  = Trigger::Software;

        return selection;
    }

    if (is_app_valid())
    {
        selection.firmware = FwType::Application;
        selection.trigger  = Trigger::None;

        return selection;
    }

    selection.firmware = FwType::Bootloader;
    selection.trigger  = Trigger::InvalidApp;

    return selection;
}

bool fw_selector::FwSelector::is_app_valid()
{
    uint32_t   stack_pointer = 0;
    uint32_t   reset_vector  = 0;
    const auto app_info      = _hwa.app_info();

    if ((app_info.app.end <= app_info.app.start) ||
        !is_app_vector_valid(stack_pointer, reset_vector) ||
        !find_app_validation_record())
    {
        return false;
    }

    return true;
}

bool fw_selector::FwSelector::is_app_vector_valid(uint32_t& stack_pointer, uint32_t& reset_vector)
{
    std::array<uint32_t, 2> vector_table = {};
    const auto              app_info     = _hwa.app_info();

    if (!_hwa.read_app(app_info.vector_table_offset,
                       std::span<uint8_t>(reinterpret_cast<uint8_t*>(vector_table.data()), sizeof(vector_table))))
    {
        return false;
    }

    stack_pointer = vector_table[0];
    reset_vector  = vector_table[1];

    return is_aligned(stack_pointer, VALIDATION_SP_ALIGNMENT_BYTES) &&
           is_in_range(stack_pointer, app_info.sram.start, app_info.sram.end) &&
           ((reset_vector & THUMB_BIT_MASK) != 0U) &&
           is_in_range(reset_vector & ~THUMB_BIT_MASK, app_info.app.start, app_info.app.end);
}

bool fw_selector::FwSelector::find_app_validation_record()
{
    const auto     app_info = _hwa.app_info();
    const uint32_t app_size = app_info.app.end - app_info.app.start;

    if ((app_info.app.end <= app_info.app.start) ||
        (app_size < IMAGE_VALIDATION_RECORD_SIZE))
    {
        return false;
    }

    std::array<uint8_t, FLASH_SCAN_BLOCK_SIZE> data          = {};
    uint32_t                                   record_offset = app_size - IMAGE_VALIDATION_RECORD_SIZE;

    while (true)
    {
        const uint32_t scan_end  = record_offset + IMAGE_VALIDATION_RECORD_SIZE;
        const uint32_t scan_size = std::min<uint32_t>(data.size(), scan_end);
        const uint32_t scan_base = scan_end - scan_size;

        if (!_hwa.read_app(scan_base, std::span<uint8_t>(data.data(), scan_size)))
        {
            return false;
        }

        while (true)
        {
            const uint32_t within_scan = record_offset - scan_base;
            const uint8_t* record_data = data.data() + within_scan;
            const uint32_t magic       = read_le32(record_data);

            if (magic == IMAGE_VALIDATION_MAGIC)
            {
                const uint32_t record_size = read_le32(record_data + sizeof(uint32_t));
                const uint32_t record_crc  = read_le32(record_data + (sizeof(uint32_t) * 2U));
                uint32_t       calculated_crc;

                if ((record_size <= record_offset) &&
                    ((record_offset - record_size) < VALIDATION_RECORD_ALIGNMENT_SIZE) &&
                    calculate_app_crc(record_size, calculated_crc) &&
                    (calculated_crc == record_crc))
                {
                    return true;
                }
            }

            if (record_offset < VALIDATION_RECORD_ALIGNMENT_SIZE)
            {
                return false;
            }

            record_offset -= VALIDATION_RECORD_ALIGNMENT_SIZE;

            if ((record_offset < scan_base) ||
                ((record_offset + IMAGE_VALIDATION_RECORD_SIZE) > scan_end))
            {
                break;
            }
        }
    }
}

bool fw_selector::FwSelector::calculate_app_crc(const uint32_t size, uint32_t& crc)
{
    std::array<uint8_t, FLASH_SCAN_BLOCK_SIZE> data   = {};
    uint32_t                                   offset = 0;

    crc = 0;

    while (offset < size)
    {
        const uint32_t bytes_to_read = std::min<uint32_t>(data.size(), size - offset);

        if (!_hwa.read_app(offset, std::span<uint8_t>(data.data(), bytes_to_read)))
        {
            return false;
        }

        crc = crc32_ieee_update(crc, data.data(), bytes_to_read);
        offset += bytes_to_read;
    }

    return true;
}
