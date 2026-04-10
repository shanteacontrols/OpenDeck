zephyr_app_library_named(cinfo)

zephyr_library_sources(
    cinfo.cpp
)

target_link_libraries(cinfo
    PUBLIC
    database
)
