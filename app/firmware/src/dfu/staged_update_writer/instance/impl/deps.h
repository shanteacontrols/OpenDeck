/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/impl/deps.h"

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Flash access used by the staged-update writer.
     */
    using Hwa = opendeck::common::dfu::flash_area::Hwa;
}    // namespace opendeck::firmware::dfu::staged_update_writer
