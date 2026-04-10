zephyr_app_library_named(indicators)

target_include_directories(indicators
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/indicators.cpp
)
