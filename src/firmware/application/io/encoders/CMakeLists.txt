add_library(encoders)

target_sources(encoders
    PRIVATE
    encoders.cpp
)

target_include_directories(encoders
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(encoders
    board_application
    database
)

target_link_libraries(encoders
    PUBLIC
    board_application
    libmidi
    database
)