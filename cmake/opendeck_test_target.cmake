#
# This helper makes unit and integration tests follow the same target selection
# model as the normal firmware build:
# - the makefile exports TARGET
# - TARGET selects boards/opendeck/<target>/opendeck.overlay
# - that overlay declares the upstream Zephyr board through zephyr-board
# - the Zephyr board maps to boards/zephyr/<board>/
#
# The helper then prepares the test build by:
# - validating that the requested target exists
# - checking whether the target allows host/hw tests
# - inferring the matching Zephyr board layer
# - resolving the logical EmuEEPROM page size from board config or partitions
# - generating a tiny CONF_FILE fragment for that page size
# - appending the correct test and OpenDeck overlays
#

include($ENV{ZEPHYR_PROJECT}/cmake/helpers.cmake)

if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "OpenDeck tests require TARGET to be exported by the makefile")
endif()

set(OPENDECK_TARGET "$ENV{TARGET}" CACHE STRING "OpenDeck topology target")

set(opendeck_testcase_yaml "${CMAKE_CURRENT_SOURCE_DIR}/testcase.yaml")
set(opendeck_metadata_query_script "$ENV{ZEPHYR_PROJECT}/scripts/query_test_metadata.sh")

if(NOT EXISTS "${opendeck_testcase_yaml}")
    message(FATAL_ERROR "OpenDeck tests require testcase.yaml at ${opendeck_testcase_yaml}")
endif()

execute_process(
    COMMAND dasel -n -m -p yaml --plain -f "${opendeck_testcase_yaml}" "tests.-"
    OUTPUT_VARIABLE opendeck_testcase_names
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if("${opendeck_testcase_names}" STREQUAL "" OR "${opendeck_testcase_names}" STREQUAL "null")
    message(FATAL_ERROR "Unable to read OpenDeck test names from ${opendeck_testcase_yaml}.")
endif()

string(REPLACE "\r\n" "\n" opendeck_testcase_names "${opendeck_testcase_names}")
string(REPLACE "\n" ";" opendeck_testcase_name_list "${opendeck_testcase_names}")
list(GET opendeck_testcase_name_list 0 opendeck_testcase_name)

execute_process(
    COMMAND bash "${opendeck_metadata_query_script}" testcase --file "${opendeck_testcase_yaml}" --name "${opendeck_testcase_name}" --key mode
    OUTPUT_VARIABLE OPENDECK_TEST_MODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT OPENDECK_TEST_MODE STREQUAL "host" AND NOT OPENDECK_TEST_MODE STREQUAL "hw")
    message(FATAL_ERROR
        "Unable to infer OpenDeck test mode from ${opendeck_testcase_yaml}. "
        "Expected a 'host' or 'hw' tag."
    )
endif()

function(opendeck_test_apply_target)
    set(firmware_overlay_path "$ENV{ZEPHYR_PROJECT}/app/boards/opendeck/${OPENDECK_TARGET}/opendeck.overlay")

    if(NOT EXISTS "${firmware_overlay_path}")
        message(FATAL_ERROR "Target '${OPENDECK_TARGET}' is missing OpenDeck overlay at ${firmware_overlay_path}.")
    endif()

    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --target "${OPENDECK_TARGET}" --key test_host
        OUTPUT_VARIABLE target_host_test_enabled
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --target "${OPENDECK_TARGET}" --key test_hw
        OUTPUT_VARIABLE target_hw_test_enabled
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --target "${OPENDECK_TARGET}" --key zephyr_board
        OUTPUT_VARIABLE zephyr_board
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if("${OPENDECK_TEST_MODE}" STREQUAL "host")
        if(NOT target_host_test_enabled STREQUAL "true")
            message(FATAL_ERROR "Target '${OPENDECK_TARGET}' is disabled for host tests in ${firmware_overlay_path}.")
        endif()
    elseif("${OPENDECK_TEST_MODE}" STREQUAL "hw")
        if(NOT target_hw_test_enabled STREQUAL "true")
            message(FATAL_ERROR "Target '${OPENDECK_TARGET}' is disabled for hardware tests in ${firmware_overlay_path}.")
        endif()
    else()
        message(FATAL_ERROR "Unknown OPENDECK_TEST_MODE '${OPENDECK_TEST_MODE}'. Expected 'host' or 'hw'.")
    endif()

    if(zephyr_board STREQUAL "")
        message(FATAL_ERROR
            "Target '${OPENDECK_TARGET}' does not define zephyr-board in ${firmware_overlay_path}."
        )
    endif()

    string(REPLACE "/" "_" zephyr_board_dir "${zephyr_board}")
    set(zephyr_overlay_dir "$ENV{ZEPHYR_PROJECT}/app/boards/zephyr/${zephyr_board_dir}")
    set(partitions_overlay_path "${zephyr_overlay_dir}/partitions.overlay")
    set(firmware_conf_path "${zephyr_overlay_dir}/firmware.conf")
    set(target_firmware_overlay_path "$ENV{ZEPHYR_PROJECT}/app/boards/opendeck/${OPENDECK_TARGET}/firmware.overlay")

    if(NOT EXISTS "${partitions_overlay_path}")
        message(FATAL_ERROR "Target '${OPENDECK_TARGET}' is missing partitions overlay at ${partitions_overlay_path}.")
    endif()

    set(emueeprom_page_size "")

    # Tests need the logical EmuEEPROM page size to match runtime behavior.
    # Prefer an explicit board firmware.conf override, then fall back to the
    # first EEPROM backing partition size from partitions.overlay.
    if(EXISTS "${firmware_conf_path}")
        set(emueeprom_page_size_from_conf "")
        read_config_value("${firmware_conf_path}" "CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE" emueeprom_page_size_from_conf)

        if(NOT emueeprom_page_size_from_conf STREQUAL "")
            set(emueeprom_page_size "${emueeprom_page_size_from_conf}")
        endif()
    endif()

    if(emueeprom_page_size STREQUAL "")
        file(STRINGS "${partitions_overlay_path}" partitions_overlay_lines)
        set(in_emueeprom_page1_partition OFF)

        foreach(line IN LISTS partitions_overlay_lines)
            string(STRIP "${line}" line)

            if(line MATCHES "^emueeprom_page1_partition:")
                set(in_emueeprom_page1_partition ON)
                continue()
            endif()

            if(NOT in_emueeprom_page1_partition)
                continue()
            endif()

            if(line MATCHES "^};")
                set(in_emueeprom_page1_partition OFF)
                continue()
            endif()

            if(line MATCHES "reg[ \t]*=[ \t]*<0x[0-9A-Fa-f]+[ \t]+0x([0-9A-Fa-f]+)>;")
                math(EXPR emueeprom_page_size "0x${CMAKE_MATCH_1}")
                break()
            elseif(line MATCHES "reg[ \t]*=[ \t]*<0x[0-9A-Fa-f]+[ \t]+DT_SIZE_K\\(([0-9]+)\\)>;")
                math(EXPR emueeprom_page_size "${CMAKE_MATCH_1} * 1024")
                break()
            elseif(line MATCHES "reg[ \t]*=[ \t]*<0x[0-9A-Fa-f]+[ \t]+DT_SIZE_M\\(([0-9]+)\\)>;")
                math(EXPR emueeprom_page_size "${CMAKE_MATCH_1} * 1024 * 1024")
                break()
            endif()
        endforeach()
    endif()

    if(emueeprom_page_size STREQUAL "")
        message(FATAL_ERROR "Unable to infer CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE from ${partitions_overlay_path}.")
    endif()

    set(target_conf_file "${CMAKE_CURRENT_BINARY_DIR}/opendeck_test_target.conf")
    file(WRITE "${target_conf_file}" "CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE=${emueeprom_page_size}\n")

    # Apply the board-side emulation overlay first, then any target-specific
    # test aliases, and finally the OpenDeck target overlay.
    list(APPEND CONF_FILE "${target_conf_file}")
    set(test_overlay_path "${zephyr_overlay_dir}/test.overlay")

    if(EXISTS "${test_overlay_path}")
        list(APPEND DTC_OVERLAY_FILE "${test_overlay_path}")
    endif()

    if(EXISTS "${target_firmware_overlay_path}")
        # Host tests build against native_sim, not the real target board DTS.
        # The shared OpenDeck overlay still contains abstract resource
        # references such as:
        #
        #   &opendeck_din_midi {
        #       uart = <&opendeck_uart_din_midi>;
        #   };
        #
        # So reuse the alias declarations from that target's firmware.overlay
        # instead of maintaining a separate target-specific host-test alias file.
        # For example, a target firmware.overlay may contain:
        #
        #   opendeck_uart_din_midi: &uart0 {};
        #   opendeck_uart_touchscreen: &uart1 {};
        #   opendeck_i2c_display: &i2c1 {};
        #
        # and this block will generate opendeck_test_target.overlay with those
        # same lines. The shared board-side test.overlay still provides any
        # common emulated backing devices and partition labels that native_sim
        # would otherwise be missing.
        file(STRINGS "${target_firmware_overlay_path}" target_alias_overlay_lines
             REGEX "^opendeck_(uart_din_midi|uart_touchscreen|i2c_display):")

        if(target_alias_overlay_lines)
            set(generated_target_test_overlay
                "${CMAKE_CURRENT_BINARY_DIR}/opendeck_test_target.overlay")
            file(WRITE "${generated_target_test_overlay}" "")

            foreach(target_alias_overlay_line IN LISTS target_alias_overlay_lines)
                file(APPEND "${generated_target_test_overlay}" "${target_alias_overlay_line}\n")
            endforeach()

            list(APPEND DTC_OVERLAY_FILE "${generated_target_test_overlay}")
        endif()
    endif()

    list(APPEND DTC_OVERLAY_FILE "${firmware_overlay_path}")

    set(CONF_FILE "${CONF_FILE}" PARENT_SCOPE)
    set(DTC_OVERLAY_FILE "${DTC_OVERLAY_FILE}" PARENT_SCOPE)
endfunction()
