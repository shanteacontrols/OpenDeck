/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_cap1188/instance/impl/common.h"
#include "firmware/src/io/i2c/peripherals/sensor_cap1188/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/io/i2c/instance/impl/deps.h"

namespace opendeck::firmware::io::i2c::sensor_cap1188
{
    using Database = database::User<database::Config::Section::I2c>;
    using Hwa      = HwaPeripheral;
}    // namespace opendeck::firmware::io::i2c::sensor_cap1188
