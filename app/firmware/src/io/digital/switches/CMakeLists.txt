include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_digital_switches)

target_link_libraries(opendeck_firmware_io_digital_switches
    PUBLIC
    opendeck_firmware_util_configurable
    opendeck_firmware_database
)

if(CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES)
    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/switches.cpp
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/mapper.cpp
    )
endif()
