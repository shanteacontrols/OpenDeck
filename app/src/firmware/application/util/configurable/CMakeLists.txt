zephyr_app_library_named(configurable)

target_include_directories(configurable
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/configurable.cpp
)
