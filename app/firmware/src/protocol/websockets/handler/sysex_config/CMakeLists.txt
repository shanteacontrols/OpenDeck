include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_websockets_handler_sysex_config)

target_link_libraries(opendeck_firmware_protocol_websockets_handler_sysex_config
    PUBLIC
    kernel
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/sysex_config_handler.cpp
)
