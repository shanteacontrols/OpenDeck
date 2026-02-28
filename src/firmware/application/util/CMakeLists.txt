add_library(util)

target_include_directories(util
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

target_sources(util
    PRIVATE
    cinfo/cinfo.cpp
    configurable/configurable.cpp
    scheduler/scheduler.cpp
)

add_dependencies(util
    board_application
)

target_link_libraries(util
    PUBLIC
    board_application
)