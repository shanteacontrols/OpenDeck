if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "OpenDeck sysbuild requires TARGET to be exported by the makefile")
endif()

set(TARGET "$ENV{TARGET}")

set(opendeck_target_common_overlay ${APP_DIR}/boards/opendeck/${TARGET}/common.overlay)
set(opendeck_target_firmware_overlay ${APP_DIR}/boards/opendeck/${TARGET}/firmware.overlay)
set(opendeck_target_bootloader_overlay ${APP_DIR}/boards/opendeck/${TARGET}/bootloader.overlay)

if(NOT EXISTS ${opendeck_target_firmware_overlay})
    message(FATAL_ERROR "Missing OpenDeck firmware overlay for target '${TARGET}': ${opendeck_target_firmware_overlay}")
endif()

if(NOT EXISTS ${opendeck_target_bootloader_overlay})
    message(FATAL_ERROR "Missing OpenDeck bootloader overlay for target '${TARGET}': ${opendeck_target_bootloader_overlay}")
endif()

if(NOT DEFINED BOARD OR "${BOARD}" STREQUAL "")
    message(FATAL_ERROR "OpenDeck sysbuild requires BOARD to be set before loading target '${TARGET}'")
endif()

set(opendeck_zephyr_board "${BOARD}")

if(DEFINED BOARD_QUALIFIERS AND NOT "${BOARD_QUALIFIERS}" STREQUAL "")
    string(APPEND opendeck_zephyr_board "/${BOARD_QUALIFIERS}")
endif()

string(REPLACE "/" "_" opendeck_zephyr_board_dir "${opendeck_zephyr_board}")

set(opendeck_zephyr_board_dir_path  ${APP_DIR}/boards/zephyr/${opendeck_zephyr_board_dir})

if(NOT EXISTS ${opendeck_zephyr_board_dir_path})
    message(FATAL_ERROR "Missing Zephyr board layer for target '${TARGET}': ${opendeck_zephyr_board_dir_path}")
endif()

set(opendeck_board_common_overlay               ${opendeck_zephyr_board_dir_path}/common.overlay)
set(opendeck_board_common_conf                  ${opendeck_zephyr_board_dir_path}/common.conf)
set(opendeck_board_firmware_overlay             ${opendeck_zephyr_board_dir_path}/firmware.overlay)
set(opendeck_board_firmware_conf                ${opendeck_zephyr_board_dir_path}/firmware.conf)
set(opendeck_board_mcuboot_conf                 ${opendeck_zephyr_board_dir_path}/mcuboot.conf)
set(opendeck_common_firmware_conf               ${APP_DIR}/firmware/common.conf)
set(opendeck_firmware_release_conf              ${APP_DIR}/firmware/release.conf)
set(opendeck_firmware_debug_conf                ${APP_DIR}/firmware/debug.conf)
set(opendeck_target_firmware_conf               ${APP_DIR}/boards/opendeck/${TARGET}/firmware.conf)
set(opendeck_target_bootloader_conf             ${APP_DIR}/boards/opendeck/${TARGET}/bootloader.conf)
set(opendeck_target_network_conf                ${APP_DIR}/boards/opendeck/${TARGET}/network.conf)
set(opendeck_board_bootloader_overlay           ${opendeck_zephyr_board_dir_path}/bootloader.overlay)
set(opendeck_board_bootloader_conf              ${opendeck_zephyr_board_dir_path}/bootloader.conf)
set(opendeck_common_bootloader_conf             ${APP_DIR}/bootloader/common.conf)
set(opendeck_bootloader_release_conf            ${APP_DIR}/bootloader/release.conf)
set(opendeck_bootloader_debug_conf              ${APP_DIR}/bootloader/debug.conf)
set(opendeck_bootloader_firmware_loader_overlay ${APP_DIR}/bootloader/firmware_loader.overlay)
set(opendeck_mcuboot_overlay                    ${APP_DIR}/sysbuild/mcuboot.overlay)
set(opendeck_mcuboot_conf                       ${APP_DIR}/sysbuild/mcuboot.conf)
set(opendeck_target_mcuboot_conf                ${APP_DIR}/boards/opendeck/${TARGET}/mcuboot.conf)
set(opendeck_common_network_conf                ${APP_DIR}/common/network.conf)
set(opendeck_board_network_conf                 ${opendeck_zephyr_board_dir_path}/network.conf)
set(opendeck_board_usb_conf                     ${opendeck_zephyr_board_dir_path}/usb.conf)
set(opendeck_common_usb_conf                    ${APP_DIR}/common/usb.conf)
set(opendeck_firmware_usb_conf                  ${APP_DIR}/firmware/usb.conf)
set(opendeck_firmware_ble_conf                  ${APP_DIR}/firmware/ble.conf)
set(opendeck_bootloader_usb_conf                ${APP_DIR}/bootloader/usb.conf)
set(opendeck_generated_dir                      ${CMAKE_BINARY_DIR}/generated)
set(opendeck_metadata_query_script              $ENV{ZENV_PROJECT_ROOT}/scripts/query_metadata.sh)

function(opendeck_read_target_overlay_value overlay_file key output_var)
    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --overlay "${overlay_file}" --key "${key}"
        OUTPUT_VARIABLE overlay_value
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(${output_var} "${overlay_value}" PARENT_SCOPE)
endfunction()

set(opendeck_shared_conf_files)

if(DEFINED CONF_FILE AND NOT "${CONF_FILE}" STREQUAL "")
    set(opendeck_shared_conf_files ${CONF_FILE})
endif()

if("${ZENV_BUILD_TYPE}" STREQUAL "release")
    set(opendeck_firmware_mode_conf   ${opendeck_firmware_release_conf})
    set(opendeck_bootloader_mode_conf ${opendeck_bootloader_release_conf})
elseif("${ZENV_BUILD_TYPE}" STREQUAL "debug")
    set(opendeck_firmware_mode_conf   ${opendeck_firmware_debug_conf})
    set(opendeck_bootloader_mode_conf ${opendeck_bootloader_debug_conf})
else()
    message(FATAL_ERROR "OpenDeck sysbuild requires ZENV_BUILD_TYPE to be either 'release' or 'debug'")
endif()

file(MAKE_DIRECTORY ${opendeck_generated_dir})

set(opendeck_runtime_board_name "${TARGET}")

execute_process(
    COMMAND bash "${opendeck_metadata_query_script}" target --target "${TARGET}" --key board_name
    OUTPUT_VARIABLE opendeck_runtime_board_name_override
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT "${opendeck_runtime_board_name_override}" STREQUAL "")
    set(opendeck_runtime_board_name "${opendeck_runtime_board_name_override}")
endif()

set(OPENDECK_USB_MIDI_LABEL   "OpenDeck | ${opendeck_runtime_board_name}")
set(OPENDECK_USB_PRODUCT_NAME "OpenDeck | ${opendeck_runtime_board_name}")

configure_file($ENV{ZENV_PROJECT_ROOT}/cmake/usb_midi_label.overlay.in
               ${opendeck_generated_dir}/firmware_usb_midi_label.overlay
               @ONLY)

configure_file($ENV{ZENV_PROJECT_ROOT}/cmake/usb_product.conf.in
               ${opendeck_generated_dir}/firmware_usb_product.conf
               @ONLY)

set(OPENDECK_USB_PRODUCT_NAME "OpenDeck DFU | ${opendeck_runtime_board_name}")

configure_file($ENV{ZENV_PROJECT_ROOT}/cmake/usb_product.conf.in
               ${opendeck_generated_dir}/bootloader_usb_product.conf
               @ONLY)

if(NOT EXISTS ${opendeck_board_firmware_overlay})
    message(FATAL_ERROR "Missing firmware overlay for target '${TARGET}': ${opendeck_board_firmware_overlay}")
endif()

if(NOT EXISTS ${opendeck_board_firmware_conf})
    message(FATAL_ERROR "Missing firmware conf for target '${TARGET}': ${opendeck_board_firmware_conf}")
endif()

if(NOT EXISTS ${opendeck_common_firmware_conf})
    message(FATAL_ERROR "Missing common firmware conf: ${opendeck_common_firmware_conf}")
endif()

if(NOT EXISTS ${opendeck_firmware_mode_conf})
    message(FATAL_ERROR "Missing firmware ${ZENV_BUILD_TYPE} conf: ${opendeck_firmware_mode_conf}")
endif()

if(NOT EXISTS ${opendeck_board_bootloader_overlay})
    message(FATAL_ERROR "Missing bootloader overlay for target '${TARGET}': ${opendeck_board_bootloader_overlay}")
endif()

if(NOT EXISTS ${opendeck_board_bootloader_conf})
    message(FATAL_ERROR "Missing bootloader conf for target '${TARGET}': ${opendeck_board_bootloader_conf}")
endif()

if(NOT EXISTS ${opendeck_common_bootloader_conf})
    message(FATAL_ERROR "Missing common bootloader conf: ${opendeck_common_bootloader_conf}")
endif()

if(NOT EXISTS ${opendeck_bootloader_mode_conf})
    message(FATAL_ERROR "Missing bootloader ${ZENV_BUILD_TYPE} conf: ${opendeck_bootloader_mode_conf}")
endif()

if(NOT EXISTS ${opendeck_bootloader_firmware_loader_overlay})
    message(FATAL_ERROR "Missing bootloader firmware-loader overlay: ${opendeck_bootloader_firmware_loader_overlay}")
endif()

if(NOT EXISTS ${opendeck_mcuboot_overlay})
    message(FATAL_ERROR "Missing MCUboot overlay: ${opendeck_mcuboot_overlay}")
endif()

if(NOT EXISTS ${opendeck_mcuboot_conf})
    message(FATAL_ERROR "Missing MCUboot conf: ${opendeck_mcuboot_conf}")
endif()

if(NOT EXISTS ${opendeck_common_network_conf})
    message(FATAL_ERROR "Missing common network conf: ${opendeck_common_network_conf}")
endif()

if(NOT EXISTS ${opendeck_common_usb_conf})
    message(FATAL_ERROR "Missing common USB conf: ${opendeck_common_usb_conf}")
endif()

if(NOT EXISTS ${opendeck_firmware_usb_conf})
    message(FATAL_ERROR "Missing firmware USB conf: ${opendeck_firmware_usb_conf}")
endif()

if(NOT EXISTS ${opendeck_firmware_ble_conf})
    message(FATAL_ERROR "Missing firmware BLE conf: ${opendeck_firmware_ble_conf}")
endif()

if(NOT EXISTS ${opendeck_bootloader_usb_conf})
    message(FATAL_ERROR "Missing bootloader USB conf: ${opendeck_bootloader_usb_conf}")
endif()

set(opendeck_firmware_network_enabled   FALSE)
set(opendeck_bootloader_network_enabled FALSE)
set(opendeck_firmware_usb_enabled       FALSE)
set(opendeck_bootloader_usb_enabled     FALSE)
set(opendeck_firmware_ble_enabled       FALSE)

opendeck_read_target_overlay_value(${opendeck_target_firmware_overlay} transport_usb opendeck_firmware_usb_transport)
opendeck_read_target_overlay_value(${opendeck_target_firmware_overlay} transport_network opendeck_firmware_network_transport)
opendeck_read_target_overlay_value(${opendeck_target_firmware_overlay} transport_ble opendeck_firmware_ble_transport)
opendeck_read_target_overlay_value(${opendeck_target_bootloader_overlay} transport_usb opendeck_bootloader_usb_transport)
opendeck_read_target_overlay_value(${opendeck_target_bootloader_overlay} transport_network opendeck_bootloader_network_transport)

if("${opendeck_firmware_usb_transport}" STREQUAL "true")
    set(opendeck_firmware_usb_enabled TRUE)
endif()

if("${opendeck_bootloader_usb_transport}" STREQUAL "true")
    set(opendeck_bootloader_usb_enabled TRUE)
endif()

if("${opendeck_firmware_network_transport}" STREQUAL "true")
    set(opendeck_firmware_network_enabled TRUE)
endif()

if("${opendeck_bootloader_network_transport}" STREQUAL "true")
    set(opendeck_bootloader_network_enabled TRUE)
endif()

if("${opendeck_firmware_ble_transport}" STREQUAL "true")
    set(opendeck_firmware_ble_enabled TRUE)
    set(OPENDECK_BT_DEVICE_NAME "OpenDeck | ${opendeck_runtime_board_name}")

    configure_file($ENV{ZENV_PROJECT_ROOT}/cmake/bluetooth_device_name.conf.in
                   ${opendeck_generated_dir}/firmware_bluetooth_device_name.conf
                   @ONLY)
endif()

set(opendeck_firmware_extra_dtc_overlay_files)

if(EXISTS ${opendeck_board_common_overlay})
    list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_board_common_overlay})
endif()

list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_board_firmware_overlay})

if(EXISTS ${opendeck_target_common_overlay})
    list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_target_common_overlay})
endif()

list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_target_firmware_overlay})

if(opendeck_firmware_usb_enabled)
    list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_generated_dir}/firmware_usb_midi_label.overlay)
endif()

set(${DEFAULT_IMAGE}_EXTRA_DTC_OVERLAY_FILE ${opendeck_firmware_extra_dtc_overlay_files} CACHE INTERNAL "")

set(opendeck_firmware_conf_files)
list(APPEND opendeck_firmware_conf_files ${opendeck_shared_conf_files})

if(EXISTS ${opendeck_board_common_conf})
    list(APPEND opendeck_firmware_conf_files ${opendeck_board_common_conf})
endif()

list(APPEND opendeck_firmware_conf_files ${opendeck_board_firmware_conf})
list(APPEND opendeck_firmware_conf_files ${opendeck_common_firmware_conf})
list(APPEND opendeck_firmware_conf_files ${opendeck_firmware_mode_conf})

set(${DEFAULT_IMAGE}_CONF_FILE ${opendeck_firmware_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_firmware_extra_conf_files)

if(opendeck_firmware_network_enabled)
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_common_network_conf})

    if(EXISTS ${opendeck_board_network_conf})
        list(APPEND opendeck_firmware_extra_conf_files ${opendeck_board_network_conf})
    endif()

    if(EXISTS ${opendeck_target_network_conf})
        list(APPEND opendeck_firmware_extra_conf_files ${opendeck_target_network_conf})
    endif()
endif()

if(opendeck_firmware_usb_enabled)
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_common_usb_conf})

    if(EXISTS ${opendeck_board_usb_conf})
        list(APPEND opendeck_firmware_extra_conf_files ${opendeck_board_usb_conf})
    endif()

    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_firmware_usb_conf})
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_generated_dir}/firmware_usb_product.conf)
endif()

if(opendeck_firmware_ble_enabled)
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_firmware_ble_conf})
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_generated_dir}/firmware_bluetooth_device_name.conf)
endif()

if(EXISTS ${opendeck_target_firmware_conf})
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_target_firmware_conf})
endif()

set(${DEFAULT_IMAGE}_EXTRA_CONF_FILE ${opendeck_firmware_extra_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_mcuboot_extra_dtc_overlay_files)

if(EXISTS ${opendeck_board_common_overlay})
    list(APPEND opendeck_mcuboot_extra_dtc_overlay_files ${opendeck_board_common_overlay})
endif()

if(EXISTS ${opendeck_target_common_overlay})
    list(APPEND opendeck_mcuboot_extra_dtc_overlay_files ${opendeck_target_common_overlay})
endif()

list(APPEND opendeck_mcuboot_extra_dtc_overlay_files ${opendeck_target_bootloader_overlay})
list(APPEND opendeck_mcuboot_extra_dtc_overlay_files ${opendeck_mcuboot_overlay})

set(mcuboot_EXTRA_DTC_OVERLAY_FILE ${opendeck_mcuboot_extra_dtc_overlay_files} CACHE INTERNAL "" FORCE)
set(mcuboot_DTS_ROOT ${APP_DIR} CACHE INTERNAL "" FORCE)

set(opendeck_mcuboot_extra_conf_files)
list(APPEND opendeck_mcuboot_extra_conf_files ${opendeck_shared_conf_files})

if(EXISTS ${opendeck_board_common_conf})
    list(APPEND opendeck_mcuboot_extra_conf_files ${opendeck_board_common_conf})
endif()

if(EXISTS ${opendeck_board_mcuboot_conf})
    list(APPEND opendeck_mcuboot_extra_conf_files ${opendeck_board_mcuboot_conf})
endif()

if(EXISTS ${opendeck_target_mcuboot_conf})
    list(APPEND opendeck_mcuboot_extra_conf_files ${opendeck_target_mcuboot_conf})
endif()

list(APPEND opendeck_mcuboot_extra_conf_files ${opendeck_mcuboot_conf})

set(mcuboot_EXTRA_CONF_FILE ${opendeck_mcuboot_extra_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_bootloader_extra_dtc_overlay_files)

if(EXISTS ${opendeck_board_common_overlay})
    list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_board_common_overlay})
endif()

list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_board_bootloader_overlay})
list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_bootloader_firmware_loader_overlay})

if(EXISTS ${opendeck_target_common_overlay})
    list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_target_common_overlay})
endif()

list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_target_bootloader_overlay})

set(opendeck_bootloader_EXTRA_DTC_OVERLAY_FILE ${opendeck_bootloader_extra_dtc_overlay_files} CACHE INTERNAL "" FORCE)
set(opendeck_bootloader_conf_files)
list(APPEND opendeck_bootloader_conf_files ${opendeck_shared_conf_files})

if(EXISTS ${opendeck_board_common_conf})
    list(APPEND opendeck_bootloader_conf_files ${opendeck_board_common_conf})
endif()

list(APPEND opendeck_bootloader_conf_files ${opendeck_board_bootloader_conf})
list(APPEND opendeck_bootloader_conf_files ${opendeck_common_bootloader_conf})
list(APPEND opendeck_bootloader_conf_files ${opendeck_bootloader_mode_conf})

set(opendeck_bootloader_CONF_FILE ${opendeck_bootloader_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_bootloader_extra_conf_files)

if(opendeck_bootloader_network_enabled)
    list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_common_network_conf})

    if(EXISTS ${opendeck_board_network_conf})
        list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_board_network_conf})
    endif()

    if(EXISTS ${opendeck_target_network_conf})
        list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_target_network_conf})
    endif()
endif()

if(opendeck_bootloader_usb_enabled)
    list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_common_usb_conf})

    if(EXISTS ${opendeck_board_usb_conf})
        list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_board_usb_conf})
    endif()

    list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_bootloader_usb_conf})
    list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_generated_dir}/bootloader_usb_product.conf)
endif()

if(EXISTS ${opendeck_target_bootloader_conf})
    list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_target_bootloader_conf})
endif()

set(opendeck_bootloader_EXTRA_CONF_FILE ${opendeck_bootloader_extra_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_board_sysbuild_cmake ${opendeck_zephyr_board_dir_path}/sysbuild.cmake)

if(EXISTS ${opendeck_board_sysbuild_cmake})
    include(${opendeck_board_sysbuild_cmake})
endif()

function(opendeck_add_merged_artifacts)
    set(opendeck_merged_hex ${CMAKE_BINARY_DIR}/merged.hex)
    set(opendeck_merged_bin ${CMAKE_BINARY_DIR}/merged.bin)
    set(opendeck_merged_uf2 ${CMAKE_BINARY_DIR}/merged.uf2)
    set(opendeck_merged_outputs ${opendeck_merged_hex} ${opendeck_merged_bin})

    set(opendeck_merged_hex_inputs)

    sysbuild_get(opendeck_mcuboot_dir IMAGE mcuboot VAR APPLICATION_BINARY_DIR CACHE)
    sysbuild_get(opendeck_app_signed_hex IMAGE ${DEFAULT_IMAGE} VAR BYPRODUCT_KERNEL_SIGNED_HEX_NAME CACHE)
    sysbuild_get(opendeck_bootloader_signed_hex IMAGE opendeck_bootloader VAR BYPRODUCT_KERNEL_SIGNED_HEX_NAME CACHE)
    sysbuild_get(opendeck_has_uf2 IMAGE ${DEFAULT_IMAGE} VAR CONFIG_BUILD_OUTPUT_UF2 KCONFIG)
    sysbuild_get(opendeck_flash_base IMAGE ${DEFAULT_IMAGE} VAR CONFIG_FLASH_BASE_ADDRESS KCONFIG)
    sysbuild_get(opendeck_uf2_use_flash_base IMAGE ${DEFAULT_IMAGE} VAR CONFIG_BUILD_OUTPUT_UF2_USE_FLASH_BASE KCONFIG)
    sysbuild_get(opendeck_uf2_use_flash_offset IMAGE ${DEFAULT_IMAGE} VAR CONFIG_BUILD_OUTPUT_UF2_USE_FLASH_OFFSET KCONFIG)
    sysbuild_get(opendeck_uf2_family IMAGE ${DEFAULT_IMAGE} VAR CONFIG_BUILD_OUTPUT_UF2_FAMILY_ID KCONFIG)

    list(APPEND opendeck_merged_hex_inputs
        ${opendeck_mcuboot_dir}/zephyr/zephyr.hex
        ${opendeck_bootloader_signed_hex}
        ${opendeck_app_signed_hex}
        ${opendeck_extra_merged_hex_files}
    )

    if(DEFINED opendeck_uf2_family)
        string(REGEX REPLACE "^\"([^\"]*)\"$" "\\1" opendeck_uf2_family "${opendeck_uf2_family}")
    endif()

    add_custom_command(
        OUTPUT
        ${opendeck_merged_hex}

        COMMAND
        ${PYTHON_EXECUTABLE}
        $ENV{ZEPHYR_BASE}/scripts/build/mergehex.py
        -o ${opendeck_merged_hex}
        --overlap replace
        ${opendeck_merged_hex_inputs}

        DEPENDS
        mcuboot
        opendeck_bootloader
        ${DEFAULT_IMAGE}
        ${opendeck_extra_merged_image_targets}
        ${opendeck_merged_hex_inputs}

        COMMENT
        "Generating merged OpenDeck HEX image"

        VERBATIM
    )

    add_custom_command(
        OUTPUT
        ${opendeck_merged_bin}

        COMMAND
        bash
        $ENV{ZENV_PROJECT_ROOT}/scripts/merge_bin.sh
        --build-dir ${CMAKE_BINARY_DIR}

        DEPENDS
        ${opendeck_merged_hex}
        $ENV{ZENV_PROJECT_ROOT}/scripts/merge_bin.sh
        $ENV{ZENV_PROJECT_ROOT}/scripts/query_metadata.sh

        COMMENT
        "Generating merged OpenDeck BIN image"

        VERBATIM
    )

    if(opendeck_has_uf2)
        if(NOT DEFINED opendeck_uf2_family)
            message(FATAL_ERROR "UF2 output is enabled, but CONFIG_BUILD_OUTPUT_UF2_FAMILY_ID is unavailable")
        endif()

        list(APPEND opendeck_merged_outputs ${opendeck_merged_uf2})

        if(NOT DEFINED opendeck_flash_base OR "${opendeck_flash_base}" STREQUAL "")
            message(FATAL_ERROR "UF2 output is enabled, but unable to resolve UF2 base address")
        endif()

        add_custom_command(
            OUTPUT
            ${opendeck_merged_uf2}

            COMMAND
            ${PYTHON_EXECUTABLE}
            $ENV{ZEPHYR_BASE}/scripts/build/uf2conv.py
            -c
            -b ${opendeck_flash_base}
            -f ${opendeck_uf2_family}
            -o ${opendeck_merged_uf2}
            ${opendeck_merged_bin}

            DEPENDS
            ${opendeck_merged_bin}

            COMMENT
            "Generating merged OpenDeck UF2 image"

            VERBATIM
        )
    endif()

    add_custom_target(opendeck_merged_artifacts ALL
        DEPENDS
        ${opendeck_merged_outputs}
    )
endfunction()

cmake_language(DEFER CALL opendeck_add_merged_artifacts)
