cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap()

opendeck_test_add_modules(
    opendeck_common_metadata            "app/common/src/metadata"
    opendeck_firmware_database          "app/firmware/src/database"
    opendeck_firmware_util_configurable "app/firmware/src/util/configurable"
    opendeck_firmware_io_i2c            "app/firmware/src/io/i2c"
)

target_sources(app
    PRIVATE
    firmware_io_i2c_sensor_apds9960_test.cpp
    firmware_io_i2c_sensor_bno085_test.cpp
    firmware_io_i2c_sensor_cap1188_test.cpp
    firmware_io_i2c_sensor_vl53l4cx_test.cpp
)
