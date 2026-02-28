add_library(leds)

target_sources(leds
    PRIVATE
    leds.cpp
)

target_include_directories(leds
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(leds
    board_application
    mcu
    database
)

target_link_libraries(leds
    PUBLIC
    board_application
    mcu
    libmidi
    database
)