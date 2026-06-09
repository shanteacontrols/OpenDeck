/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/staged_update/hwa/hw/hwa_hw.h"

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Zephyr flash-map backend for staged DFU storage.
     */
    using HwaHw = opendeck::common::dfu::staged_update::HwaHw;
}    // namespace opendeck::firmware::dfu::staged_update_writer
