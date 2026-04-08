add_executable(database)

target_sources(database
    PRIVATE
    test.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/database.cpp
    $ENV{ZEPHYR_PROJECT}/app/src/firmware/application/database/custom_init.cpp
)

target_link_libraries(database
    PUBLIC
    common
)

add_test(
    NAME database
    COMMAND $<TARGET_FILE:database>
)