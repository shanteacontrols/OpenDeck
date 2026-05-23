include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_database)

target_link_libraries(opendeck_firmware_database
    PUBLIC
    opendeck_firmware_util_configurable
    opendeck_common_metadata
    zlibs_utils_lessdb
    zlibs_utils_misc
    zlibs_utils_sysex_conf
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/custom_init.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/database.cpp
)
