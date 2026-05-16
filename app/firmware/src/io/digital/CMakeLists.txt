include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(digital)


target_link_libraries(digital
    PUBLIC
    switches
    encoders
    zlibs-utils-threads
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/digital.cpp
)
