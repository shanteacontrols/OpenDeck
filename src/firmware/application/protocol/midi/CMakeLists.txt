add_library(midi)

target_sources(midi
    PRIVATE
    midi.cpp
)

target_include_directories(midi
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/liblessdb/include
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

add_dependencies(midi
    board_application
)

target_link_libraries(midi
    PUBLIC
    board_application
    libmidi
)