include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_database)


target_link_libraries(opendeck_firmware_database
    PUBLIC
    opendeck_firmware_util_configurable
    opendeck_common_metadata
    zlibs-utils-lessdb
    zlibs-utils-misc
    zlibs-utils-sysex_conf
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/custom_init.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/database.cpp
)
