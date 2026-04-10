zephyr_app_library_named(digital)

target_include_directories(digital
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(digital
    PUBLIC
    buttons
    encoders
    zlibs-utils-threads
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/digital.cpp
)
