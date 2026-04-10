zephyr_app_library_named(protocol_midi)

target_include_directories(protocol_midi
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_link_libraries(protocol_midi
    PUBLIC
    database
    configurable
    zlibs-utils-midi
)

if(NOT OPENDECK_TEST)
    target_link_libraries(protocol_midi
        PUBLIC
        zlibs-drivers-usb_hw
        zlibs-drivers-uart_hw
    )
endif()

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/midi.cpp
)
