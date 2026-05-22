include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(flash_stream_writer)

target_sources(flash_stream_writer PRIVATE
    instance/impl/flash_stream_writer.cpp
)
