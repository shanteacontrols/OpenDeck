include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_touchscreen)

target_link_libraries(opendeck_firmware_io_touchscreen
    PUBLIC
    opendeck_firmware_database
    opendeck_firmware_util_configurable
)

if(CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN)
    if(CONFIG_HW_BUILD)
        add_subdirectory(hwa/hw)

        target_link_libraries(opendeck_firmware_io_touchscreen
            PUBLIC
            opendeck_firmware_io_touchscreen_hwa_hw
        )
    endif()

    zephyr_library_sources(
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/touchscreen.cpp
        ${CMAKE_CURRENT_LIST_DIR}/instance/impl/models/nextion/nextion.cpp
    )
endif()
