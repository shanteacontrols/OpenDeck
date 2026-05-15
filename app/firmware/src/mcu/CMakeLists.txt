include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(mcu)


target_link_libraries(mcu
    PUBLIC
    zlibs-utils-misc
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/mcu.cpp
)
