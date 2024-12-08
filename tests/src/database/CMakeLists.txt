if(NOT "PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(database)

    target_sources(database
        PRIVATE
        test.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/custom_init.cpp
    )

    target_link_libraries(database
        PUBLIC
        common
    )

    add_test(
        NAME database
        COMMAND $<TARGET_FILE:database>
    )
endif()