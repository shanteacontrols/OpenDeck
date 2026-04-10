zephyr_app_library_named(touchscreen)

target_include_directories(touchscreen
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(touchscreen
    PUBLIC
    database
    configurable
    zlibs-drivers-uart_hw
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/touchscreen.cpp
    ${CMAKE_CURRENT_LIST_DIR}/models/nextion/nextion.cpp
)
