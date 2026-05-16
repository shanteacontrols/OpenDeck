include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_webconfig_hwa_hw)

target_link_libraries(protocol_webconfig_hwa_hw
    PUBLIC
    mcu
    protocol_webconfig
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/hwa_hw.cpp
)
