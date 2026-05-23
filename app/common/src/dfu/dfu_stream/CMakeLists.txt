include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_common_dfu_dfu_stream)

target_link_libraries(opendeck_common_dfu_dfu_stream
    PUBLIC
    opendeck_common_metadata
)

target_sources(opendeck_common_dfu_dfu_stream PRIVATE
    instance/impl/dfu_stream.cpp
)
