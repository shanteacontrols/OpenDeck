include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(analog)


target_link_libraries(analog
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/analog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mapper.cpp
)
