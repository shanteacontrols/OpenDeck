add_library(buttons)

target_sources(buttons
    PRIVATE
    buttons.cpp
)

target_include_directories(buttons
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(buttons
    board_application
    mcu
    database
)

target_link_libraries(buttons
    PUBLIC
    board_application
    mcu
    libmidi
    database
)