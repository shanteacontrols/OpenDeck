include($ENV{ZENV_PROJECT_ROOT}/cmake/opendeck_library.cmake)
opendeck_library_named(opendeck_firmware_io_i2c_peripherals_sensor_vl53l4cx TYPE OBJECT)

target_include_directories(opendeck_firmware_io_i2c_peripherals_sensor_vl53l4cx
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/port
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src
)

target_sources(opendeck_firmware_io_i2c_peripherals_sensor_vl53l4cx
    PRIVATE
    sensor_vl53l4cx.cpp
    port/vl53l4cx_port.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_api.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_api_calibration.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_api_core.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_api_debug.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_api_preset_modes.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_core.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_core_support.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_dmax.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_hist_algos_gen3.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_hist_algos_gen4.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_hist_char.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_hist_core.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_hist_funcs.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_nvm.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_platform_ipp.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_register_funcs.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_sigma_estimate.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_silicon_core.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_wait.cpp
    $ENV{ZEPHYR_WS}/modules/vl53l4cx/src/vl53l4cx_xtalk.cpp
)
