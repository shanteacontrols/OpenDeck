if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(midi)

    target_sources(midi
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/Database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/CustomInit.cpp
        ${PROJECT_ROOT}/src/firmware/application/protocol/midi/MIDI.cpp
    )

    target_link_libraries(midi
        PUBLIC
        common
    )

    add_test(
        NAME midi
        COMMAND $<TARGET_FILE:midi>
    )
endif()