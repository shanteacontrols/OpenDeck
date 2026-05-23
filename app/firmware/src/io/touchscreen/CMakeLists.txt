include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_touchscreen)

target_link_libraries(opendeck_firmware_io_touchscreen
    PUBLIC
    opendeck_firmware_database
    opendeck_firmware_util_configurable
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/touchscreen.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/models/nextion/nextion.cpp
)
