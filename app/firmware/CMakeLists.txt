add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(src/mcu)
add_subdirectory(src/database)
add_subdirectory(src/util/configurable)
add_subdirectory(src/util/cinfo)
add_subdirectory(src/protocol/midi)
add_subdirectory(src/protocol/osc)
add_subdirectory(src/protocol/webconfig)
add_subdirectory(src/protocol/mdns)
add_subdirectory(src/staged_update)
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
add_subdirectory(src/protocol/webconfig/hwa/hw)
add_subdirectory(src/protocol/mdns/hwa/hw)
add_subdirectory(src/io/touchscreen/hwa/hw)

set(OPENDECK_FIRMWARE_LIBS
    metadata
    mcu
    database
    configurable
    cinfo
    protocol_midi
    protocol_osc
    protocol_webconfig
    protocol_mdns
    staged_update
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
    protocol_webconfig_hwa_hw
    protocol_mdns_hwa_hw
    touchscreen_hwa_hw
)

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
