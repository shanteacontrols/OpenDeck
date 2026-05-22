cmake_minimum_required(VERSION 3.27.0)
list(APPEND DTS_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(opendeck_bootloader)

zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/flash_stream_writer ${CMAKE_CURRENT_BINARY_DIR}/flash_stream_writer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/dfu_stream ${CMAKE_CURRENT_BINARY_DIR}/dfu_stream)
add_subdirectory(src/indicators)
add_subdirectory(src/direct_update_writer)
add_subdirectory(src/signaling)
add_subdirectory(src/system)
add_subdirectory(src/webusb)

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_WEBSOCKETS)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/websockets/firmware_upload ${CMAKE_CURRENT_BINARY_DIR}/websockets_firmware_upload)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/websockets/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/websockets_hwa_hw)
    add_subdirectory(src/websockets)
    add_subdirectory(src/websockets/hwa/hw)
endif()

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/mdns ${CMAKE_CURRENT_BINARY_DIR}/mdns)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/src/mdns/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/mdns_hwa_hw)
    add_subdirectory(src/mdns)
    add_subdirectory(src/mdns/hwa/hw)
    add_subdirectory(src/mdns/services/hw)
endif()

if(CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE)
    add_subdirectory(src/staged_update_reader)
endif()

target_link_libraries(app
    PRIVATE
    metadata
    dfu_stream
    opendeck_bootloader_indicators
    opendeck_bootloader_direct_update_writer
    opendeck_bootloader_signaling
    opendeck_bootloader_system
    opendeck_bootloader_webusb
)

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_WEBSOCKETS)
    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_websockets
        opendeck_bootloader_websockets_hwa_hw
    )
endif()

if(CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE)
    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_staged_update_reader
    )
endif()

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS)
    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_mdns
        opendeck_bootloader_mdns_hwa_hw
        opendeck_bootloader_mdns_services_hw
    )
endif()

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU)
    add_subdirectory(src/webusb/hwa/hw)
endif()

if(CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU)
    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_webusb_hwa_hw
    )
endif()

target_sources(app
    PRIVATE
    src/main.cpp
)
