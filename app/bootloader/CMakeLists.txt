cmake_minimum_required(VERSION 3.27.0)
list(APPEND DTS_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(opendeck_bootloader)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common/metadata ${CMAKE_CURRENT_BINARY_DIR}/metadata)
add_subdirectory(src/fw_selector)
add_subdirectory(src/indicators)
add_subdirectory(src/updater)

if(CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE)
    add_subdirectory(src/staged_update)
endif()

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/fw_selector
    ${CMAKE_CURRENT_LIST_DIR}/src/indicators
    ${CMAKE_CURRENT_LIST_DIR}/src/updater
    ${CMAKE_CURRENT_LIST_DIR}/src/webusb
    ${CMAKE_CURRENT_LIST_DIR}/../common
    ${CMAKE_CURRENT_LIST_DIR}/../firmware/src
)

target_link_libraries(app
    PRIVATE
    metadata
    opendeck_bootloader_fw_selector
    opendeck_bootloader_indicators
    opendeck_bootloader_updater
    zlibs-drivers-usb_hw
)

if(CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE)
    target_link_libraries(app
        PRIVATE
        opendeck_bootloader_staged_update
    )
endif()

target_sources(app
    PRIVATE
    src/main.cpp
    src/webusb/transport.cpp
)
