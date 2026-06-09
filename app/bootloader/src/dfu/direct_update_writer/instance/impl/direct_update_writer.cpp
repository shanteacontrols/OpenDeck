/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include <zephyr/logging/log_ctrl.h>

#include <span>

using namespace opendeck::bootloader;

dfu::direct_update_writer::DirectUpdateWriter::DirectUpdateWriter(opendeck::common::dfu::flash_area::Hwa&               flash_area,
                                                                  opendeck::bootloader::dfu::direct_update_writer::Hwa& hwa)
    : DfuWriter(flash_area)
    , _hwa(hwa)
{}

bool dfu::direct_update_writer::DirectUpdateWriter::commit(const opendeck::common::dfu::dfu_stream_parser::Header& header,
                                                           const uint32_t                                          expected_size)
{
    if (!DfuWriter::commit(header, expected_size))
    {
        return false;
    }

    status("Firmware update complete, rebooting");

    _hwa.apply();

    LOG_PANIC();

    return true;
}
