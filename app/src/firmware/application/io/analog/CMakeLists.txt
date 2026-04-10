zephyr_app_library_named(analog)

target_include_directories(analog
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(analog
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/analog.cpp
)
