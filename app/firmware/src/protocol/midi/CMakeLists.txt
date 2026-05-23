include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_midi)

target_link_libraries(opendeck_firmware_protocol_midi
    PUBLIC
    opendeck_firmware_database
    opendeck_firmware_util_configurable
    zlibs-utils-midi
    kernel
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/midi.cpp
)
