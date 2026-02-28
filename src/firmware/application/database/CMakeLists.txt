add_library(database)

target_sources(database
    PRIVATE
    custom_init.cpp
    database.cpp
)

target_include_directories(database
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(database
    board_application
    mcu
    liblessdb
)

target_link_libraries(database
    PUBLIC
    board_application
    mcu
    liblessdb
)