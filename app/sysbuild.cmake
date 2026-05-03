if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "OpenDeck sysbuild requires TARGET to be exported by the makefile")
endif()

set(TARGET "$ENV{TARGET}")

set(opendeck_board_overlay ${APP_DIR}/boards/opendeck/${TARGET}/opendeck.overlay)

if(NOT EXISTS ${opendeck_board_overlay})
    message(FATAL_ERROR "Missing OpenDeck board overlay for target '${TARGET}': ${opendeck_board_overlay}")
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

set(opendeck_board_partitions_overlay   ${opendeck_zephyr_board_dir_path}/partitions.overlay)
set(opendeck_board_firmware_overlay     ${opendeck_zephyr_board_dir_path}/firmware.overlay)
set(opendeck_board_firmware_conf        ${opendeck_zephyr_board_dir_path}/firmware.conf)
set(opendeck_common_firmware_conf       ${APP_DIR}/firmware/firmware.conf)
set(opendeck_target_firmware_overlay    ${APP_DIR}/boards/opendeck/${TARGET}/firmware.overlay)
set(opendeck_target_firmware_conf       ${APP_DIR}/boards/opendeck/${TARGET}/firmware.conf)
set(opendeck_target_bootloader_overlay  ${APP_DIR}/boards/opendeck/${TARGET}/bootloader.overlay)
set(opendeck_target_bootloader_conf     ${APP_DIR}/boards/opendeck/${TARGET}/bootloader.conf)
set(opendeck_board_bootloader_overlay   ${opendeck_zephyr_board_dir_path}/bootloader.overlay)
set(opendeck_board_bootloader_conf      ${opendeck_zephyr_board_dir_path}/bootloader.conf)
set(opendeck_common_bootloader_conf     ${APP_DIR}/bootloader/bootloader.conf)
set(opendeck_generated_dir              ${CMAKE_BINARY_DIR}/generated)
set(opendeck_metadata_query_script      $ENV{ZEPHYR_PROJECT}/scripts/query_metadata.sh)

function(opendeck_read_config_value config_file variable_name output_var)
    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" config --file "${config_file}" --key "${variable_name}"
        OUTPUT_VARIABLE config_value
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(${output_var} "${config_value}" PARENT_SCOPE)
endfunction()

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

configure_file($ENV{ZEPHYR_PROJECT}/cmake/usb_midi_label.overlay.in
               ${opendeck_generated_dir}/firmware_usb_midi_label.overlay
               @ONLY)

configure_file($ENV{ZEPHYR_PROJECT}/cmake/usb_product.conf.in
               ${opendeck_generated_dir}/firmware_usb_product.conf
               @ONLY)

set(OPENDECK_USB_PRODUCT_NAME "OpenDeck DFU | ${opendeck_runtime_board_name}")

configure_file($ENV{ZEPHYR_PROJECT}/cmake/usb_product.conf.in
               ${opendeck_generated_dir}/bootloader_usb_product.conf
               @ONLY)

if(NOT EXISTS ${opendeck_board_partitions_overlay})
    message(FATAL_ERROR "Missing partitions overlay for target '${TARGET}': ${opendeck_board_partitions_overlay}")
endif()

if(NOT EXISTS ${opendeck_board_firmware_overlay})
    message(FATAL_ERROR "Missing firmware overlay for target '${TARGET}': ${opendeck_board_firmware_overlay}")
endif()

if(NOT EXISTS ${opendeck_board_firmware_conf})
    message(FATAL_ERROR "Missing firmware conf for target '${TARGET}': ${opendeck_board_firmware_conf}")
endif()

if(NOT EXISTS ${opendeck_common_firmware_conf})
    message(FATAL_ERROR "Missing common firmware conf: ${opendeck_common_firmware_conf}")
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

set(opendeck_shared_conf_files)

if(DEFINED CONF_FILE AND NOT "${CONF_FILE}" STREQUAL "")
    set(opendeck_shared_conf_files ${CONF_FILE})
endif()

set(opendeck_ble_enabled            FALSE)
set(opendeck_bt_enabled             FALSE)
set(opendeck_bt_peripheral_enabled  FALSE)

opendeck_read_config_value(${opendeck_board_firmware_conf}   CONFIG_BT               opendeck_board_bt_enabled)
opendeck_read_config_value(${opendeck_board_firmware_conf}   CONFIG_BT_PERIPHERAL    opendeck_board_bt_peripheral_enabled)
opendeck_read_config_value(${opendeck_target_firmware_conf}  CONFIG_BT               opendeck_target_bt_enabled)
opendeck_read_config_value(${opendeck_target_firmware_conf}  CONFIG_BT_PERIPHERAL    opendeck_target_bt_peripheral_enabled)

if("${opendeck_board_bt_enabled}" STREQUAL "y" OR "${opendeck_target_bt_enabled}" STREQUAL "y")
    set(opendeck_bt_enabled TRUE)
endif()

if("${opendeck_board_bt_peripheral_enabled}" STREQUAL "y" OR "${opendeck_target_bt_peripheral_enabled}" STREQUAL "y")
    set(opendeck_bt_peripheral_enabled TRUE)
endif()

foreach(opendeck_shared_conf_file ${opendeck_shared_conf_files})
    opendeck_read_config_value(${opendeck_shared_conf_file} CONFIG_BT opendeck_shared_bt_enabled)
    opendeck_read_config_value(${opendeck_shared_conf_file} CONFIG_BT_PERIPHERAL opendeck_shared_bt_peripheral_enabled)

    if("${opendeck_shared_bt_enabled}" STREQUAL "y")
        set(opendeck_bt_enabled TRUE)
    endif()

    if("${opendeck_shared_bt_peripheral_enabled}" STREQUAL "y")
        set(opendeck_bt_peripheral_enabled TRUE)
    endif()
endforeach()

if(opendeck_bt_enabled AND opendeck_bt_peripheral_enabled)
    set(opendeck_ble_enabled TRUE)
    set(OPENDECK_BT_DEVICE_NAME "OpenDeck | ${opendeck_runtime_board_name}")

    configure_file($ENV{ZEPHYR_PROJECT}/cmake/bluetooth_device_name.conf.in
                   ${opendeck_generated_dir}/firmware_bluetooth_device_name.conf
                   @ONLY)
endif()

set(opendeck_firmware_extra_dtc_overlay_files)
list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_board_partitions_overlay})
list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_board_firmware_overlay})
list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_board_overlay})
list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_generated_dir}/firmware_usb_midi_label.overlay)

if(EXISTS ${opendeck_target_firmware_overlay})
    list(APPEND opendeck_firmware_extra_dtc_overlay_files ${opendeck_target_firmware_overlay})
endif()

set(${DEFAULT_IMAGE}_EXTRA_DTC_OVERLAY_FILE ${opendeck_firmware_extra_dtc_overlay_files} CACHE INTERNAL "")

set(opendeck_firmware_conf_files)
list(APPEND opendeck_firmware_conf_files ${opendeck_shared_conf_files})
list(APPEND opendeck_firmware_conf_files ${opendeck_common_firmware_conf})

set(${DEFAULT_IMAGE}_CONF_FILE ${opendeck_firmware_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_firmware_extra_conf_files)
list(APPEND opendeck_firmware_extra_conf_files ${opendeck_board_firmware_conf})
list(APPEND opendeck_firmware_extra_conf_files ${opendeck_generated_dir}/firmware_usb_product.conf)

if(opendeck_ble_enabled)
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_generated_dir}/firmware_bluetooth_device_name.conf)
endif()

if(EXISTS ${opendeck_target_firmware_conf})
    list(APPEND opendeck_firmware_extra_conf_files ${opendeck_target_firmware_conf})
endif()

set(${DEFAULT_IMAGE}_EXTRA_CONF_FILE    ${opendeck_firmware_extra_conf_files} CACHE INTERNAL "" FORCE)
set(${DEFAULT_IMAGE}_SIGNING_SCRIPT     $ENV{ZEPHYR_PROJECT}/cmake/app_runner_files.cmake CACHE INTERNAL "" FORCE)

set(opendeck_bootloader_extra_dtc_overlay_files)
list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_board_partitions_overlay})
list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_board_bootloader_overlay})
list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_board_overlay})

if(EXISTS ${opendeck_target_bootloader_overlay})
    list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_target_bootloader_overlay})
elseif(EXISTS ${opendeck_target_firmware_overlay})
    set(opendeck_generated_bootloader_alias_overlay ${opendeck_generated_dir}/bootloader_target_alias.overlay)

    # Most targets only need the shared opendeck.overlay to resolve abstract
    # OpenDeck resource labels during bootloader DTS parsing. The bootloader
    # does not normally use these buses at runtime, but opendeck.overlay still
    # contains properties such as:
    #
    #   &opendeck_din_midi {
    #       uart = <&opendeck_uart_din_midi>;
    #   };
    #
    # So if a target has no real bootloader-specific DTS changes, reuse the
    # alias declarations from the target firmware.overlay instead of keeping a
    # separate alias-only app/boards/opendeck/<target>/bootloader.overlay.
    # For example, a target firmware.overlay may contain:
    #
    #   opendeck_uart_din_midi: &uart1 {};
    #   opendeck_uart_touchscreen: &uart1 {};
    #   opendeck_i2c_display: &i2c1 {};
    #
    # and this block will generate bootloader_target_alias.overlay with those
    # same lines. Targets with real bootloader-specific DTS changes still
    # provide their own bootloader.overlay and bypass this fallback.
    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --target "${TARGET}" --key target_alias_overlay_line
        OUTPUT_VARIABLE opendeck_bootloader_alias_lines
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT opendeck_bootloader_alias_lines STREQUAL "")
        file(WRITE ${opendeck_generated_bootloader_alias_overlay} "${opendeck_bootloader_alias_lines}\n")

        list(APPEND opendeck_bootloader_extra_dtc_overlay_files ${opendeck_generated_bootloader_alias_overlay})
    endif()
endif()

set(opendeck_bootloader_EXTRA_DTC_OVERLAY_FILE  ${opendeck_bootloader_extra_dtc_overlay_files} CACHE INTERNAL "" FORCE)
set(opendeck_bootloader_conf_files)
list(APPEND opendeck_bootloader_conf_files ${opendeck_shared_conf_files})
list(APPEND opendeck_bootloader_conf_files ${opendeck_common_bootloader_conf})

set(opendeck_bootloader_CONF_FILE ${opendeck_bootloader_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_bootloader_extra_conf_files)
list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_board_bootloader_conf})
list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_generated_dir}/bootloader_usb_product.conf)

if(EXISTS ${opendeck_target_bootloader_conf})
    list(APPEND opendeck_bootloader_extra_conf_files ${opendeck_target_bootloader_conf})
endif()

set(opendeck_bootloader_EXTRA_CONF_FILE ${opendeck_bootloader_extra_conf_files} CACHE INTERNAL "" FORCE)

set(opendeck_board_sysbuild_cmake ${opendeck_zephyr_board_dir_path}/sysbuild.cmake)

if(EXISTS ${opendeck_board_sysbuild_cmake}) 
    include(${opendeck_board_sysbuild_cmake})
endif()

ExternalZephyrProject_Add(
  APPLICATION opendeck_bootloader
  SOURCE_DIR ${APP_DIR}/bootloader
)

add_dependencies(${DEFAULT_IMAGE} opendeck_bootloader)
sysbuild_add_dependencies(FLASH ${DEFAULT_IMAGE} opendeck_bootloader)

function(opendeck_add_merged_artifacts)
    set(opendeck_firmware_validated_hex_file ${CMAKE_BINARY_DIR}/${DEFAULT_IMAGE}/app_validated.hex)
    set(opendeck_merged_app_hex_file         ${CMAKE_BINARY_DIR}/app_validated.hex)
    set(opendeck_bootloader_hex_file         ${CMAKE_BINARY_DIR}/opendeck_bootloader/zephyr/zephyr.hex)
    set(opendeck_merged_hex_inputs           ${opendeck_bootloader_hex_file}
                                             ${opendeck_merged_app_hex_file}
                                             ${opendeck_extra_merged_hex_files})
    set(opendeck_merged_hex                  ${CMAKE_BINARY_DIR}/merged.hex)
    set(opendeck_merged_bin                  ${CMAKE_BINARY_DIR}/merged.bin)
    set(opendeck_merged_uf2                  ${CMAKE_BINARY_DIR}/merged.uf2)
    set(opendeck_merged_outputs              ${opendeck_merged_hex}
                                             ${opendeck_merged_bin})

    sysbuild_get(opendeck_has_uf2    IMAGE opendeck_bootloader VAR CONFIG_BUILD_OUTPUT_UF2 KCONFIG)
    sysbuild_get(opendeck_uf2_base   IMAGE opendeck_bootloader VAR CONFIG_FLASH_BASE_ADDRESS KCONFIG)
    sysbuild_get(opendeck_uf2_family IMAGE opendeck_bootloader VAR CONFIG_BUILD_OUTPUT_UF2_FAMILY_ID KCONFIG)
    sysbuild_get(opendeck_objcopy    IMAGE opendeck_bootloader VAR CMAKE_OBJCOPY CACHE)

    if(DEFINED opendeck_uf2_family)
        string(REGEX REPLACE "^\"([^\"]*)\"$" "\\1" opendeck_uf2_family "${opendeck_uf2_family}")
    endif()

    add_custom_command(
        OUTPUT
        ${opendeck_merged_app_hex_file}

        COMMAND
        ${CMAKE_COMMAND}
        -E copy
        ${opendeck_firmware_validated_hex_file}
        ${opendeck_merged_app_hex_file}

        DEPENDS
        ${DEFAULT_IMAGE}

        COMMENT
        "Copying validated application HEX image"

        VERBATIM
    )

    add_custom_command(
        OUTPUT
        ${opendeck_merged_hex}

        COMMAND
        $ENV{ZEPHYR_BASE}/scripts/build/mergehex.py
        -o ${opendeck_merged_hex}
        ${opendeck_merged_hex_inputs}

        DEPENDS
        ${opendeck_merged_hex_inputs}

        COMMENT
        "Generating merged OpenDeck HEX image"

        VERBATIM
    )

    add_custom_command(
        OUTPUT
        ${opendeck_merged_bin}

        COMMAND
        ${opendeck_objcopy}
        -I ihex
        --gap-fill 0xFF
        -O binary
        ${opendeck_merged_hex}
        ${opendeck_merged_bin}

        DEPENDS
        ${opendeck_merged_hex}

        COMMENT
        "Generating merged OpenDeck BIN image"

        VERBATIM
    )

    if(opendeck_has_uf2)
        if(NOT DEFINED opendeck_uf2_base)
            message(FATAL_ERROR "UF2 output is enabled, but CONFIG_FLASH_BASE_ADDRESS is unavailable")
        endif()

        if(NOT DEFINED opendeck_uf2_family)
            message(FATAL_ERROR "UF2 output is enabled, but CONFIG_BUILD_OUTPUT_UF2_FAMILY_ID is unavailable")
        endif()

        list(APPEND opendeck_merged_outputs ${opendeck_merged_uf2})

        add_custom_command(
            OUTPUT
            ${opendeck_merged_uf2}

            COMMAND
            $ENV{ZEPHYR_BASE}/scripts/build/uf2conv.py
            -c
            -b ${opendeck_uf2_base}
            -f ${opendeck_uf2_family}
            -o ${opendeck_merged_uf2}
            ${opendeck_merged_bin}

            DEPENDS
            ${opendeck_merged_bin}

            COMMENT
            "Generating merged bootloader and application UF2 image"

            VERBATIM
        )
    endif()

    add_custom_target(opendeck_merged_uf2_target ALL
        DEPENDS
        ${opendeck_merged_outputs}
    )

    add_dependencies(opendeck_merged_uf2_target
        ${DEFAULT_IMAGE}
        opendeck_bootloader
        ${opendeck_extra_merged_image_targets}
    )
endfunction()

cmake_language(DEFER CALL opendeck_add_merged_artifacts)
