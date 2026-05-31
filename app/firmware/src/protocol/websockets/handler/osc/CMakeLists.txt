include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_websockets_handler_osc)

if(CONFIG_PROJECT_TARGET_SUPPORT_OSC)
    target_link_libraries(opendeck_firmware_protocol_websockets_handler_osc
        PUBLIC
        opendeck_firmware_protocol_osc_packet
    )

    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/osc_handler.cpp
    )
endif()
