include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_websockets_hwa_hw)

target_link_libraries(protocol_websockets_hwa_hw
    PUBLIC
    mcu
    opendeck_websockets_hwa_hw
    protocol_websockets
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/hwa_hw.cpp
)
