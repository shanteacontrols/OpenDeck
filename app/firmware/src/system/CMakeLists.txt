include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
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
)

if(CONFIG_PROJECT_TARGET_SUPPORT_MIDI)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_protocol_midi)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_io_digital)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_io_digital_switches)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_io_digital_encoders)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_ADC)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_io_analog)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_io_outputs)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_io_touchscreen)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_I2C)
    target_link_libraries(opendeck_firmware_system PUBLIC opendeck_firmware_io_i2c)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    target_link_libraries(opendeck_firmware_system
        PUBLIC
        opendeck_firmware_protocol_mdns
    )
endif()
