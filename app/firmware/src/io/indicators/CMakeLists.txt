include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(indicators)


zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/indicators.cpp
)
