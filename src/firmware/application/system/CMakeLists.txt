add_library(system)

target_sources(system
    PRIVATE
    system.cpp
)

target_include_directories(system
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(system
    board_application
)

target_link_libraries(system
    PUBLIC
    board_application
    libmidi
    libsysexconf
    version
    database
    util
)