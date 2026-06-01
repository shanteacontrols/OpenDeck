include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_bootloader_protocols_websockets_hwa_hw)

add_subdirectory($ENV{ZENV_PROJECT_ROOT}/app/common/src/protocols/websockets/hwa/hw ${CMAKE_CURRENT_BINARY_DIR}/websockets_hwa_hw)

target_link_libraries(opendeck_bootloader_protocols_websockets_hwa_hw
    PUBLIC
    opendeck_common_protocols_websockets_hwa_hw
)

target_sources(opendeck_bootloader_protocols_websockets_hwa_hw PRIVATE
    hwa_hw.cpp
)
