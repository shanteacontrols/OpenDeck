include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_common_protocols_mdns_hwa_hw)

target_link_libraries(opendeck_common_protocols_mdns_hwa_hw
    PUBLIC
    opendeck_common_protocols_mdns
    zlibs_utils_misc
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/hwa_hw.cpp
)
