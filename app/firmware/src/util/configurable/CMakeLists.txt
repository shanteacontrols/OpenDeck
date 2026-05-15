include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(configurable)


zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/configurable.cpp
)
