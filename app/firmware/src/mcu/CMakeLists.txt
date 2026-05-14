include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(mcu)

target_include_directories(mcu
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
    $ENV{ZEPHYR_PROJECT}/app/common
    $ENV{ZEPHYR_PROJECT}/app/bootloader/src
)

target_link_libraries(mcu
    PUBLIC
    zlibs-utils-misc
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/mcu.cpp
)
