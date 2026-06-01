include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_common_protocols_websockets)

target_link_libraries(opendeck_common_protocols_websockets
    PUBLIC
    kernel
    opendeck_common_protocols_websockets_handler
    zlibs_utils_misc
    zlibs_utils_threads
)

target_sources(opendeck_common_protocols_websockets PRIVATE
    instance/impl/websockets.cpp
)
