include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(staged_update_writer)


target_link_libraries(staged_update_writer
    PUBLIC
    flash_stream_writer
    metadata
    kernel
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/staged_update_writer.cpp
)
