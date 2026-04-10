zephyr_app_library_named(buttons)

target_include_directories(buttons
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(buttons
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/buttons.cpp
)
