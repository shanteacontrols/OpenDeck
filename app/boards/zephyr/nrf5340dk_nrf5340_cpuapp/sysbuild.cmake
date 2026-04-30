set(opendeck_net_core_image hci_ipc)
set(opendeck_net_core_source ${ZEPHYR_BASE}/samples/bluetooth/${opendeck_net_core_image})

ExternalZephyrProject_Add(
  APPLICATION ${opendeck_net_core_image}
  SOURCE_DIR  ${opendeck_net_core_source}
  BOARD       nrf5340dk/nrf5340/cpunet
)

set(${opendeck_net_core_image}_EXTRA_CONF_FILE
    ${CMAKE_CURRENT_LIST_DIR}/hci_ipc.conf
    CACHE INTERNAL "" FORCE)

sysbuild_add_dependencies(FLASH ${DEFAULT_IMAGE} ${opendeck_net_core_image})

list(APPEND opendeck_extra_merged_hex_files      ${CMAKE_BINARY_DIR}/${opendeck_net_core_image}/zephyr/zephyr.hex)
list(APPEND opendeck_extra_merged_image_targets  ${opendeck_net_core_image})
