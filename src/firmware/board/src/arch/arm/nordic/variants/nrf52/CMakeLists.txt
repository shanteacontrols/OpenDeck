set(NRF52_SDK_PATH ${WORKSPACE_ROOT}/modules/libcore/deps/nrf52840/nRF5_SDK_17.1.0_ddde560)
set(NRF52_SOFTDEVICE_PATH ${NRF52_SDK_PATH}/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex)

target_compile_definitions(mcu
    PUBLIC
    SOFTDEVICE_PRESENT
)

list(APPEND FIRMWARE_HEX_LIST
    ${NRF52_SOFTDEVICE_PATH}
)