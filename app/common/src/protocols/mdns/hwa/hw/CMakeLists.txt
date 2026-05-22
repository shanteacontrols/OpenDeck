include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_mdns_hwa_hw)

target_link_libraries(opendeck_mdns_hwa_hw
    PUBLIC
    opendeck_mdns
    zlibs-utils-misc
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/hwa_hw.cpp
)
