/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/staged_update/shared/deps.h"

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Flash access used by the staged-update writer.
     */
    using Hwa = opendeck::common::dfu::staged_update::Hwa;
}    // namespace opendeck::firmware::dfu::staged_update_writer
