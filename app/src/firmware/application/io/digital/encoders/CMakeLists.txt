zephyr_app_library_named(encoders)

target_include_directories(encoders
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(encoders
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/encoders.cpp
)
