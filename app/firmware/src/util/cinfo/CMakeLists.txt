include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(cinfo)

zephyr_library_sources(
    cinfo.cpp
)

target_link_libraries(cinfo
    PUBLIC
    database
)
