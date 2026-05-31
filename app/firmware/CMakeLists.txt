add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/signaling ${CMAKE_CURRENT_BINARY_DIR}/signaling_common)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/mcu ${CMAKE_CURRENT_BINARY_DIR}/mcu)
add_subdirectory(src/database)
add_subdirectory(src/util/configurable)
add_subdirectory(src/util/cinfo)
add_subdirectory(src/system)

set(OPENDECK_FIRMWARE_LIBS
    opendeck_common_metadata
    opendeck_common_signaling
    opendeck_common_mcu
    opendeck_firmware_database
    opendeck_firmware_util_configurable
    opendeck_firmware_util_cinfo
    opendeck_firmware_system
)

set(OPENDECK_FIRMWARE_HWA_LIBS)

if(CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES)
    add_subdirectory(src/io/digital/switches)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_digital_switches)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS)
    add_subdirectory(src/io/digital/encoders)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_digital_encoders)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS)
    add_subdirectory(src/io/digital)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_digital)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_ADC)
    add_subdirectory(src/io/analog)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_analog)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS)
    add_subdirectory(src/io/outputs)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_outputs)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN)
    add_subdirectory(src/io/touchscreen)
    add_subdirectory(src/io/touchscreen/hwa/hw)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_touchscreen)
    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS opendeck_firmware_io_touchscreen_hwa_hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_I2C)
    add_subdirectory(src/io/i2c)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_i2c)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_TRAFFIC_INDICATORS)
    add_subdirectory(src/io/indicators)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_io_indicators)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MIDI)
    add_subdirectory(src/protocol/midi)
    add_subdirectory(src/protocol/midi/hwa/hw)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_protocol_midi)
    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS opendeck_firmware_protocol_midi_hwa_hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
    add_subdirectory(src/protocol/osc)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_protocol_osc)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    add_subdirectory(src/protocol/websockets)
    add_subdirectory(src/protocol/websockets/hwa/hw)

    list(APPEND OPENDECK_FIRMWARE_LIBS
        opendeck_firmware_protocol_websockets
    )

    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS opendeck_firmware_protocol_websockets_hwa_hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    add_subdirectory(src/protocol/mdns)
    add_subdirectory(src/protocol/mdns/services/hw)

    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_protocol_mdns)

    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS
        opendeck_firmware_protocol_mdns_services_hw
    )
endif()

target_sources(app
    PRIVATE
    ../common/src/atomic_compat.cpp
    src/main.cpp
)

target_link_libraries(app
    PRIVATE
    ${OPENDECK_FIRMWARE_LIBS}
    ${OPENDECK_FIRMWARE_HWA_LIBS}
)
