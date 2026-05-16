include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_library.cmake)
opendeck_library_named(staged_update)


target_link_libraries(staged_update
    PUBLIC
    metadata
    kernel
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/instance/impl/staged_update.cpp
)
