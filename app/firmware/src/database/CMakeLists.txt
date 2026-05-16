include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(database)


target_link_libraries(database
    PUBLIC
    configurable
    metadata
    zlibs-utils-lessdb
    zlibs-utils-misc
    zlibs-utils-sysex_conf
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/custom_init.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/database.cpp
)
