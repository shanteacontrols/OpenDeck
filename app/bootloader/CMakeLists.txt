cmake_minimum_required(VERSION 3.27.0)
list(APPEND DTS_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(opendeck_bootloader)

zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/signaling ${CMAKE_CURRENT_BINARY_DIR}/signaling_common)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/dfu/flash_stream_writer ${CMAKE_CURRENT_BINARY_DIR}/flash_stream_writer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/dfu/dfu_stream_parser ${CMAKE_CURRENT_BINARY_DIR}/dfu_stream_parser)
add_subdirectory(src/io/indicators)
add_subdirectory(src/dfu/direct_update_writer)
add_subdirectory(src/signaling)
add_subdirectory(src/system)

target_link_libraries(app
    PRIVATE
    opendeck_common_metadata
    opendeck_common_signaling
    opendeck_common_dfu_dfu_stream_parser
    opendeck_bootloader_io_indicators
    opendeck_bootloader_dfu_direct_update_writer
    opendeck_bootloader_signaling
    opendeck_bootloader_system
)

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU)
    add_subdirectory(src/protocols/webusb)

    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_protocols_webusb
    )
endif()

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_WEBSOCKETS)
    add_subdirectory(src/protocols/websockets)

    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_protocols_websockets
    )
endif()

if(CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE)
    add_subdirectory(src/dfu/staged_update_reader)

    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_dfu_staged_update_reader
    )
endif()

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS)
    add_subdirectory(src/protocols/mdns)

    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_protocols_mdns
    )
endif()

target_sources(app
    PRIVATE
    ../common/src/atomic_compat.cpp
    src/main.cpp
)
