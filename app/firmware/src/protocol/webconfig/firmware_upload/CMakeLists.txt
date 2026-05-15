include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(protocol_webconfig_firmware_upload)


target_link_libraries(protocol_webconfig_firmware_upload
    PUBLIC
    staged_update
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/firmware_upload.cpp
)
