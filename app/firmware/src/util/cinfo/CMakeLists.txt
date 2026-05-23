include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_util_cinfo)

zephyr_library_sources(
    cinfo.cpp
)

target_link_libraries(opendeck_firmware_util_cinfo
    PUBLIC
    opendeck_firmware_database
)
