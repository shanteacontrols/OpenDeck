include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_i2c_peripherals_sensor_vl53l5cx TYPE OBJECT)

target_include_directories(opendeck_firmware_io_i2c_peripherals_sensor_vl53l5cx
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/port
    $ENV{ZEPHYR_WS}/modules/vl53l5cx/src
)

target_sources(opendeck_firmware_io_i2c_peripherals_sensor_vl53l5cx
    PRIVATE
    instance/impl/sensor_vl53l5cx.cpp
    instance/impl/port/vl53l5cx_port.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l5cx/src/vl53l5cx_api.cpp
)
