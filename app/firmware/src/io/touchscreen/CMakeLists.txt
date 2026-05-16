include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(touchscreen)

target_link_libraries(touchscreen
    PUBLIC
    database
    configurable
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/touchscreen.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/models/nextion/nextion.cpp
)
