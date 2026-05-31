include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_common_protocols_websockets_handler)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/handler.cpp
)
