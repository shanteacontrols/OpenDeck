include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(protocol_mdns)

target_include_directories(protocol_mdns
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)

target_link_libraries(protocol_mdns
    PUBLIC
    database
    kernel
    mcu
    metadata
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/hwa_hw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mdns.cpp
)
