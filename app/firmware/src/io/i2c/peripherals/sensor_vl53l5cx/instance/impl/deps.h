/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_vl53l5cx/instance/impl/common.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l5cx/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/io/i2c/instance/impl/deps.h"

namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
{
    using Database = database::User<database::Config::Section::I2c>;
    using Hwa      = io::i2c::HwaPeripheral;
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
