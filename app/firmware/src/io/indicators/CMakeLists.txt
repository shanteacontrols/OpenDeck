include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_indicators)

if(CONFIG_PROJECT_TARGET_SUPPORT_TRAFFIC_INDICATORS)
    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/indicators.cpp
    )
endif()
