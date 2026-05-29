add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/signaling ${CMAKE_CURRENT_BINARY_DIR}/signaling_common)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/dfu/flash_stream_writer ${CMAKE_CURRENT_BINARY_DIR}/flash_stream_writer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/mcu ${CMAKE_CURRENT_BINARY_DIR}/mcu)
add_subdirectory(src/database)
add_subdirectory(src/util/configurable)
add_subdirectory(src/util/cinfo)
add_subdirectory(src/protocol/midi)
add_subdirectory(src/io/digital)
add_subdirectory(src/io/digital/switches)
add_subdirectory(src/io/digital/encoders)
add_subdirectory(src/io/analog)
add_subdirectory(src/io/outputs)
add_subdirectory(src/io/touchscreen)
add_subdirectory(src/io/i2c)
add_subdirectory(src/io/indicators)
add_subdirectory(src/system)
add_subdirectory(src/protocol/midi/hwa/hw)
add_subdirectory(src/io/touchscreen/hwa/hw)

set(OPENDECK_FIRMWARE_LIBS
    opendeck_common_metadata
    opendeck_common_signaling
    opendeck_common_dfu_flash_stream_writer
    opendeck_common_mcu
    opendeck_firmware_database
    opendeck_firmware_util_configurable
    opendeck_firmware_util_cinfo
    opendeck_firmware_protocol_midi
    opendeck_firmware_io_digital_switches
    opendeck_firmware_io_analog
    opendeck_firmware_io_outputs
    opendeck_firmware_io_digital_encoders
    opendeck_firmware_io_touchscreen
    opendeck_firmware_io_i2c
    opendeck_firmware_io_indicators
    opendeck_firmware_system
)

set(OPENDECK_FIRMWARE_HWA_LIBS
    opendeck_firmware_protocol_midi_hwa_hw
    opendeck_firmware_io_touchscreen_hwa_hw
)

if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
    add_subdirectory(src/protocol/osc)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/dfu/dfu_stream ${CMAKE_CURRENT_BINARY_DIR}/dfu_stream)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/protocols/websockets/firmware_upload ${CMAKE_CURRENT_BINARY_DIR}/websockets_firmware_upload)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/protocols/websockets/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/websockets_hwa_hw)
    add_subdirectory(src/dfu/staged_update_writer)
    add_subdirectory(src/protocol/websockets)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/protocols/mdns ${CMAKE_CURRENT_BINARY_DIR}/mdns)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/protocols/mdns/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/mdns_hwa_hw)
    add_subdirectory(src/protocol/mdns)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    add_subdirectory(src/protocol/websockets/hwa/hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    add_subdirectory(src/protocol/mdns/services/hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_protocol_osc)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    list(APPEND OPENDECK_FIRMWARE_LIBS
        opendeck_common_dfu_dfu_stream
        opendeck_common_protocols_websockets_firmware_upload
        opendeck_firmware_protocol_websockets
        opendeck_firmware_dfu_staged_update_writer
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    list(APPEND OPENDECK_FIRMWARE_LIBS opendeck_firmware_protocol_mdns)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS opendeck_firmware_protocol_websockets_hwa_hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS
        opendeck_common_protocols_mdns_hwa_hw
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
