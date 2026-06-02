/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/display/display.h"
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC
#include "firmware/src/io/i2c/peripherals/sensor_apds9960/sensor_apds9960.h"
#include "firmware/src/io/i2c/peripherals/sensor_bno085/sensor_bno085.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/sensor_vl53l4cx.h"
#endif
#include "firmware/src/database/instance/impl/database.h"

namespace opendeck::io::i2c
{
    /**
     * @brief Builder that instantiates the I2C peripheral set.
     */
    class BuilderPeripherals
    {
        public:
        /**
         * @brief Constructs the peripheral bundle around the shared I2C backend and database.
         *
         * @param hwa I2C backend used by peripherals.
         * @param database Database administrator used for peripheral configuration views.
         */
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC
        BuilderPeripherals(HwaPeripheral& hwa, database::Admin& database)
            : _sensor_apds9960_database(database)
            , _sensor_apds9960(hwa, _sensor_apds9960_database)
            , _sensor_bno085(hwa)
            , _sensor_vl53l4cx_database(database)
            , _sensor_vl53l4cx(hwa, _sensor_vl53l4cx_database)
            , _display_database(database)
            , _display(hwa, _display_database)
        {}
#else
        BuilderPeripherals(HwaPeripheral& hwa, database::Admin& database)
            : _display_database(database)
            , _display(hwa, _display_database)
        {}
#endif

        private:
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC
        sensor_apds9960::Database       _sensor_apds9960_database;
        sensor_apds9960::SensorApds9960 _sensor_apds9960;
        sensor_bno085::SensorBno085     _sensor_bno085;
        sensor_vl53l4cx::Database       _sensor_vl53l4cx_database;
        sensor_vl53l4cx::SensorVl53l4cx _sensor_vl53l4cx;
#endif
        display::Database _display_database;
        display::Display  _display;
    };
}    // namespace opendeck::io::i2c
