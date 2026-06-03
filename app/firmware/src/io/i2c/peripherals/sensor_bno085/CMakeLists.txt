include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_i2c_peripherals_sensor_bno085 TYPE OBJECT)

target_sources(opendeck_firmware_io_i2c_peripherals_sensor_bno085
    PRIVATE
    instance/impl/sensor_bno085.cpp
)
