include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
add_subdirectory(firmware_upload)

opendeck_library_named(protocol_webconfig)

target_link_libraries(protocol_webconfig
    PUBLIC
    kernel
    mcu
    protocol_webconfig_firmware_upload
)

zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_LIST_DIR}/sections-rom.ld)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/webconfig.cpp
)
