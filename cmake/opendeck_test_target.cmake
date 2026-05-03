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
# - resolving the logical EmuEEPROM page size through the metadata helper
# - exporting that page size as a test compile definition
# - appending the correct test and OpenDeck overlays
#

if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "OpenDeck tests require TARGET to be exported by the makefile")
endif()

set(OPENDECK_TARGET "$ENV{TARGET}" CACHE STRING "OpenDeck topology target")

set(opendeck_testcase_yaml "${CMAKE_CURRENT_SOURCE_DIR}/testcase.yaml")
set(opendeck_metadata_query_script "$ENV{ZEPHYR_PROJECT}/scripts/query_metadata.sh")

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

    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --target "${OPENDECK_TARGET}" --key zephyr_overlay_dir
        OUTPUT_VARIABLE zephyr_overlay_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --target "${OPENDECK_TARGET}" --key emueeprom_page_size
        OUTPUT_VARIABLE emueeprom_page_size
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND bash "${opendeck_metadata_query_script}" target --target "${OPENDECK_TARGET}" --key target_alias_overlay_line
        OUTPUT_VARIABLE target_alias_overlay_lines
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

    if(emueeprom_page_size STREQUAL "")
        message(FATAL_ERROR "Unable to infer EmuEEPROM page size for target '${OPENDECK_TARGET}'.")
    endif()

    add_compile_definitions(OPENDECK_TEST_EMUEEPROM_PAGE_SIZE=${emueeprom_page_size})

    # Apply the board-side emulation overlay first, then any target-specific
    # test aliases, and finally the OpenDeck target overlay.
    set(test_overlay_path "${zephyr_overlay_dir}/test.overlay")

    if(EXISTS "${test_overlay_path}")
        list(APPEND DTC_OVERLAY_FILE "${test_overlay_path}")
    endif()

    if(NOT target_alias_overlay_lines STREQUAL "")
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
        set(generated_target_test_overlay "${CMAKE_CURRENT_BINARY_DIR}/opendeck_test_target.overlay")
        file(WRITE "${generated_target_test_overlay}" "${target_alias_overlay_lines}\n")

        list(APPEND DTC_OVERLAY_FILE "${generated_target_test_overlay}")
    endif()

    list(APPEND DTC_OVERLAY_FILE "${firmware_overlay_path}")

    set(CONF_FILE "${CONF_FILE}" PARENT_SCOPE)
    set(DTC_OVERLAY_FILE "${DTC_OVERLAY_FILE}" PARENT_SCOPE)
endfunction()
