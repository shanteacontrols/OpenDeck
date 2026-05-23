include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_digital_encoders)


target_link_libraries(opendeck_firmware_io_digital_encoders
    PUBLIC
    opendeck_firmware_util_configurable
    opendeck_firmware_database
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/encoders.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/mapper.cpp
)
