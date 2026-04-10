add_library(sysexgen_lib STATIC)

target_include_directories(sysexgen_lib
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    $ENV{ZEPHYR_PROJECT}/app/src/firmware
)

target_sources(sysexgen_lib
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sysexgen.cpp
)
