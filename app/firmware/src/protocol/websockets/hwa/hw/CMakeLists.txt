include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_protocol_websockets_hwa_hw)

if(CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS)
    add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/websockets_hwa_hw)

    target_link_libraries(opendeck_firmware_protocol_websockets_hwa_hw
        PUBLIC
        opendeck_common_mcu
        opendeck_common_protocols_websockets_hwa_hw
        opendeck_firmware_protocol_websockets
    )

    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/hwa_hw.cpp
    )
endif()
