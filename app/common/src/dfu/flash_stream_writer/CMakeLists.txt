include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_common_dfu_flash_stream_writer)

target_sources(opendeck_common_dfu_flash_stream_writer PRIVATE
    instance/impl/flash_stream_writer.cpp
)
