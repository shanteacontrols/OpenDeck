include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_util_configurable)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/configurable.cpp
)
