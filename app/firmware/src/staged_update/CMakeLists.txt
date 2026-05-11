include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(staged_update)

target_include_directories(staged_update
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/common
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)

target_link_libraries(staged_update
    PUBLIC
    metadata
    kernel
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/staged_update.cpp
)
