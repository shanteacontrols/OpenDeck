include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_common_dfu_writer)

target_link_libraries(opendeck_common_dfu_writer
    PUBLIC
    opendeck_common_metadata
)

target_sources(opendeck_common_dfu_writer PRIVATE
    instance/impl/dfu_writer.cpp
)
