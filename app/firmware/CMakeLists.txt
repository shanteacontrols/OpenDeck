add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(src/database)
add_subdirectory(src/util/configurable)
add_subdirectory(src/util/cinfo)
add_subdirectory(src/protocol/midi)
add_subdirectory(src/io/digital)
add_subdirectory(src/io/digital/buttons)
add_subdirectory(src/io/digital/encoders)
add_subdirectory(src/io/analog)
add_subdirectory(src/io/leds)
add_subdirectory(src/io/touchscreen)
add_subdirectory(src/io/i2c)
add_subdirectory(src/io/indicators)
add_subdirectory(src/system)

set(OPENDECK_FIRMWARE_LIBS
    metadata
    database
    configurable
    cinfo
    protocol_midi
    buttons
    analog
    leds
    encoders
    touchscreen
    i2c
    indicators
    system
)

target_sources(app
    PRIVATE
    src/atomic_compat.cpp
    src/main.cpp
)

target_link_libraries(app
    PRIVATE
    ${OPENDECK_FIRMWARE_LIBS}
)
