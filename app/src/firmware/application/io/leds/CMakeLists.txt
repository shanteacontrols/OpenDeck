zephyr_app_library_named(leds)

target_include_directories(leds
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(leds
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/leds.cpp
)
