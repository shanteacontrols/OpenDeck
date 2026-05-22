include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(dfu_stream)

target_link_libraries(dfu_stream
    PUBLIC
    metadata
)

target_sources(dfu_stream PRIVATE
    instance/impl/dfu_stream.cpp
)
