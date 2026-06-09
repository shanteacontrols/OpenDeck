cmake_minimum_required(VERSION 3.20)
include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_test_suite.cmake)
opendeck_test_bootstrap()

opendeck_test_add_modules(
    opendeck_common_metadata              "app/common/src/metadata"
    opendeck_common_dfu_writer            "app/common/src/dfu/writer"
    opendeck_common_dfu_dfu_stream_parser "app/common/src/dfu/dfu_stream_parser"
    opendeck_common_dfu_upload            "app/common/src/dfu/upload"
)

target_sources(app
    PRIVATE
    common_dfu_upload_test.cpp
)
