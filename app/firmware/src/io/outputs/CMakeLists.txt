include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_outputs)

target_link_libraries(opendeck_firmware_io_outputs
    PUBLIC
    opendeck_firmware_util_configurable
    opendeck_firmware_database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/outputs.cpp
)
