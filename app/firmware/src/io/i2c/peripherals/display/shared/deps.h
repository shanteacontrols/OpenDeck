/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/display/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/io/i2c/shared/deps.h"

namespace opendeck::io::i2c::display
{
    using Database = database::User<database::Config::Section::I2c>;
    using Hwa      = io::i2c::Hwa;
}    // namespace opendeck::io::i2c::display