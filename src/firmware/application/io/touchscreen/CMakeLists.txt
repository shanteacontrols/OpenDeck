add_library(touchscreen)

target_sources(touchscreen
    PRIVATE
    touchscreen.cpp
    models/nextion/nextion.cpp
    models/viewtech/viewtech.cpp
)

target_include_directories(touchscreen
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(touchscreen
    board_application
    mcu
    database
)

target_link_libraries(touchscreen
    PUBLIC
    board_application
    mcu
    libmidi
    database
)