include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_interface_library_named(opendeck_common_mcu)

target_link_libraries(opendeck_common_mcu
    INTERFACE
    zlibs_utils_misc
)
