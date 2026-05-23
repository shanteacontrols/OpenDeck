include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_system)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/system.cpp
)

target_link_libraries(opendeck_firmware_system
    PUBLIC
    opendeck_common_metadata
    opendeck_common_mcu
    opendeck_firmware_database
    opendeck_firmware_util_configurable
    opendeck_firmware_util_cinfo
    opendeck_firmware_protocol_midi
    opendeck_firmware_io_digital
    opendeck_firmware_io_digital_switches
    opendeck_firmware_io_analog
    opendeck_firmware_io_outputs
    opendeck_firmware_io_digital_encoders
    opendeck_firmware_io_touchscreen
    opendeck_firmware_io_i2c
)

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    target_link_libraries(opendeck_firmware_system
        PUBLIC
        opendeck_firmware_protocol_mdns
    )
endif()
