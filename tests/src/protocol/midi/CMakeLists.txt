add_executable(midi)

target_sources(midi
    PRIVATE
    test.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/database.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/custom_init.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/protocol/midi/midi.cpp
)

target_link_libraries(midi
    PUBLIC
    common
)

add_test(
    NAME midi
    COMMAND $<TARGET_FILE:midi>
)