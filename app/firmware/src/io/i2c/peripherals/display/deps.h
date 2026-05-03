/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"
#include "database/database.h"
#include "io/i2c/deps.h"

namespace opendeck::io::i2c::display
{
    using Database = database::User<database::Config::Section::I2c>;
    using Hwa      = io::i2c::Hwa;
}    // namespace opendeck::io::i2c::display