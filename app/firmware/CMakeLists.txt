add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/flash_stream_writer ${CMAKE_CURRENT_BINARY_DIR}/flash_stream_writer)
add_subdirectory(src/mcu)
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
    metadata
    flash_stream_writer
    mcu
    database
    configurable
    cinfo
    protocol_midi
    switches
    analog
    outputs
    encoders
    touchscreen
    i2c
    indicators
    system
)

set(OPENDECK_FIRMWARE_HWA_LIBS
    protocol_midi_hwa_hw
    touchscreen_hwa_hw
)

if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
    add_subdirectory(src/protocol/osc)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG)
    add_subdirectory(src/staged_update_writer)
    add_subdirectory(src/protocol/webconfig)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    add_subdirectory(src/protocol/mdns)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG)
    add_subdirectory(src/protocol/webconfig/hwa/hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    add_subdirectory(src/protocol/mdns/hwa/hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
    list(APPEND OPENDECK_FIRMWARE_LIBS protocol_osc)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG)
    list(APPEND OPENDECK_FIRMWARE_LIBS
        protocol_webconfig
        staged_update_writer
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    list(APPEND OPENDECK_FIRMWARE_LIBS protocol_mdns)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG)
    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS protocol_webconfig_hwa_hw)
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_MDNS)
    list(APPEND OPENDECK_FIRMWARE_HWA_LIBS protocol_mdns_hwa_hw)
endif()

target_sources(app
    PRIVATE
    src/atomic_compat.cpp
    src/main.cpp
)

target_link_libraries(app
    PRIVATE
    ${OPENDECK_FIRMWARE_LIBS}
    ${OPENDECK_FIRMWARE_HWA_LIBS}
)
