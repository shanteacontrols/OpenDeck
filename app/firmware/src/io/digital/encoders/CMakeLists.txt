include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(encoders)

target_include_directories(encoders
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)

target_link_libraries(encoders
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/encoders.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mapper.cpp
)
