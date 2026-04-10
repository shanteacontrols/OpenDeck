zephyr_app_library_named(database)

target_include_directories(database
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(database
    PUBLIC
    metadata
    zlibs-utils-lessdb
    zlibs-utils-misc
    zlibs-utils-sysex_conf
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/custom_init.cpp
    ${CMAKE_CURRENT_LIST_DIR}/database.cpp
)
