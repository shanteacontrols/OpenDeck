include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_websockets_firmware_upload)

target_link_libraries(opendeck_websockets_firmware_upload
    PUBLIC
    dfu_stream
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/firmware_upload.cpp
)
