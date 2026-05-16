include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(switches)


target_link_libraries(switches
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/switches.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/mapper.cpp
)
