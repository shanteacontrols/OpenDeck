include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(encoders)


target_link_libraries(encoders
    PUBLIC
    configurable
    database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/encoders.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/mapper.cpp
)
