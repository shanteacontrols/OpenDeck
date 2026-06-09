include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_common_dfu_upload)

target_link_libraries(opendeck_common_dfu_upload
    PUBLIC
    opendeck_common_dfu_dfu_stream_parser
    opendeck_common_dfu_writer
)

target_sources(opendeck_common_dfu_upload PRIVATE
    instance/impl/upload.cpp
)
