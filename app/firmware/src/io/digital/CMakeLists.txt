include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_digital)


target_link_libraries(opendeck_firmware_io_digital
    PUBLIC
    opendeck_firmware_io_digital_switches
    opendeck_firmware_io_digital_encoders
    zlibs-utils-threads
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/digital.cpp
)
