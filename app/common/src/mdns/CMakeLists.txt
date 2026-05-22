include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)

opendeck_library_named(opendeck_mdns)

target_link_libraries(opendeck_mdns
    PUBLIC
    metadata
)

target_sources(opendeck_mdns PRIVATE
    instance/impl/mdns.cpp
)
