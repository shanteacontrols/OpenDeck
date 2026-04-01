if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(midi)

    target_sources(midi
        PRIVATE
        test.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/database/database.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/database/custom_init.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/protocol/midi/midi.cpp
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