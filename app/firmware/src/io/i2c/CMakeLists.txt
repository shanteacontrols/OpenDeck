include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_i2c)

target_link_libraries(opendeck_firmware_io_i2c
    PUBLIC
    opendeck_common_metadata
    opendeck_firmware_util_configurable
    opendeck_firmware_database
)

if(CONFIG_PROJECT_TARGET_SUPPORT_I2C)
    add_subdirectory(peripherals/display)

    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/i2c.cpp
    )

    target_link_libraries(opendeck_firmware_io_i2c
        PUBLIC
        opendeck_firmware_io_i2c_peripherals_display
    )

    if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
        add_subdirectory(peripherals/sensor_apds9960)
        add_subdirectory(peripherals/sensor_bno085)
        add_subdirectory(peripherals/sensor_cap1188)
        add_subdirectory(peripherals/sensor_vl53l4cx)

        target_link_libraries(opendeck_firmware_io_i2c
            PUBLIC
            opendeck_firmware_io_i2c_peripherals_sensor_apds9960
            opendeck_firmware_io_i2c_peripherals_sensor_bno085
            opendeck_firmware_io_i2c_peripherals_sensor_cap1188
            opendeck_firmware_io_i2c_peripherals_sensor_vl53l4cx
        )
    endif()
endif()
