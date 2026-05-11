include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_webconfig_firmware_upload)

target_include_directories(protocol_webconfig_firmware_upload
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/app/firmware/src
)

target_link_libraries(protocol_webconfig_firmware_upload
    PUBLIC
    staged_update
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/firmware_upload.cpp
)
