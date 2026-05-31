include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_digital)

target_link_libraries(opendeck_firmware_io_digital
    PUBLIC
    zlibs_utils_threads
)

if(TARGET opendeck_firmware_io_digital_switches)
    target_link_libraries(opendeck_firmware_io_digital
        PUBLIC
        opendeck_firmware_io_digital_switches
    )
endif()

if(TARGET opendeck_firmware_io_digital_encoders)
    target_link_libraries(opendeck_firmware_io_digital
        PUBLIC
        opendeck_firmware_io_digital_encoders
    )
endif()

if(CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS)
    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/digital.cpp
    )
endif()
