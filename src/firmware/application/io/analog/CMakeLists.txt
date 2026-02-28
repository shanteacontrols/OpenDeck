add_library(analog)

target_sources(analog
    PRIVATE
    analog.cpp
)

target_include_directories(analog
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(analog
    board_application
    mcu
    database
)

target_link_libraries(analog
    PUBLIC
    board_application
    mcu
    libmidi
    database
)