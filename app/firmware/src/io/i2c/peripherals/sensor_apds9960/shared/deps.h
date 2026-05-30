/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_apds9960/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/io/i2c/shared/deps.h"

namespace opendeck::io::i2c::sensor_apds9960
{
    using Database = database::User<database::Config::Section::I2c>;
    using Hwa      = io::i2c::HwaPeripheral;
}    // namespace opendeck::io::i2c::sensor_apds9960
