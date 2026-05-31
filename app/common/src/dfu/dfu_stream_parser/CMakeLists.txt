include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_common_dfu_dfu_stream_parser)

target_link_libraries(opendeck_common_dfu_dfu_stream_parser
    PUBLIC
    opendeck_common_metadata
)

target_sources(opendeck_common_dfu_dfu_stream_parser PRIVATE
    instance/impl/dfu_stream_parser.cpp
)
