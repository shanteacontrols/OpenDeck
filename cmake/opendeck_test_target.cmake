#
# This helper makes unit and integration tests follow the same target selection
# model as the normal firmware build:
# - the makefile exports TARGET
# - the corresponding app build must already exist
# - the app build publishes resolved target metadata in generated/target.conf
#   and generated/target.kconfig
#
# The helper then prepares the test build by:
# - checking whether the built target allows host/hardware tests
# - letting tests use that generated config as the target fact source without
#   sourcing the firmware target-derivation Kconfig layer
#

if(NOT DEFINED ENV{TARGET} OR "$ENV{TARGET}" STREQUAL "")
    message(FATAL_ERROR "OpenDeck tests require TARGET to be exported by the makefile")
endif()

set(OPENDECK_TARGET "$ENV{TARGET}" CACHE STRING "OpenDeck topology target")

set(opendeck_testcase_yaml         "${CMAKE_CURRENT_SOURCE_DIR}/testcase.yaml")
set(opendeck_metadata_query_script "$ENV{ZENV_PROJECT_ROOT}/scripts/query_metadata.sh")
set(opendeck_app_build_dir         "$ENV{ZENV_PROJECT_ROOT}/build/app/default/release/${OPENDECK_TARGET}")
set(opendeck_app_config            "${opendeck_app_build_dir}/app/zephyr/.config")
set(opendeck_target_config         "${opendeck_app_build_dir}/generated/target.conf")
set(opendeck_target_kconfig        "${opendeck_app_build_dir}/generated/target.kconfig")

if(NOT EXISTS "${opendeck_app_config}")
    message(FATAL_ERROR
        "Tests importing target config for '${OPENDECK_TARGET}' require a built app config at ${opendeck_app_config}. "
        "Build the target first with: make TARGET=${OPENDECK_TARGET}"
    )
endif()

if(NOT EXISTS "${opendeck_target_config}")
    message(FATAL_ERROR
        "Tests importing target config for '${OPENDECK_TARGET}' require generated target config at ${opendeck_target_config}. "
        "Build the target first with: make TARGET=${OPENDECK_TARGET}"
    )
endif()

if(NOT EXISTS "${opendeck_target_kconfig}")
    message(FATAL_ERROR
        "Tests importing target config for '${OPENDECK_TARGET}' require generated target Kconfig at ${opendeck_target_kconfig}. "
        "Build the target first with: make TARGET=${OPENDECK_TARGET}"
    )
endif()

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
string(REPLACE "." "\\." opendeck_testcase_selector_name "${opendeck_testcase_name}")

execute_process(
    COMMAND dasel -n -m -p yaml --plain -f "${opendeck_testcase_yaml}" "tests.${opendeck_testcase_selector_name}.tags.[*]"
    OUTPUT_VARIABLE opendeck_testcase_tags
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(OPENDECK_TEST_MODE "unknown")
string(REPLACE "\r\n" "\n" opendeck_testcase_tags "${opendeck_testcase_tags}")
string(REPLACE "\n" ";" opendeck_testcase_tag_list "${opendeck_testcase_tags}")

foreach(opendeck_testcase_tag IN LISTS opendeck_testcase_tag_list)
    if(opendeck_testcase_tag STREQUAL "host" OR opendeck_testcase_tag STREQUAL "hardware")
        set(OPENDECK_TEST_MODE "${opendeck_testcase_tag}")
    endif()
endforeach()

if(NOT OPENDECK_TEST_MODE STREQUAL "host" AND NOT OPENDECK_TEST_MODE STREQUAL "hardware")
    message(FATAL_ERROR
        "Unable to infer OpenDeck test mode from ${opendeck_testcase_yaml}. "
        "Expected a 'host' or 'hardware' tag."
    )
endif()

function(opendeck_read_app_bool output key)
    execute_process(
        COMMAND
        bash
        "${opendeck_metadata_query_script}"
        config
        --file
        "${opendeck_app_config}"
        --key
        "${key}"
        --default
        n
        OUTPUT_VARIABLE opendeck_config_value
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )

    set(${output} "${opendeck_config_value}" PARENT_SCOPE)
endfunction()

# Imports target facts from the already-built firmware image into the native
# test build.
#
# The firmware target symbols are derived in app/firmware/Kconfig from the real
# target DTS. Many of them are hidden symbols, so a native_sim test cannot set
# them directly in prj.conf, and it also should not re-run the target DTS model
# just to learn counts such as "this target has 64 analog inputs".
#
# The app build therefore exports resolved PROJECT_TARGET_* values into
# generated/target.conf and generated/target.kconfig. This helper appends the
# generated config values and validates whether the selected target supports the
# test mode. Tests that need imported Kconfig symbols explicitly rsource
# generated/target.kconfig from their own Kconfig root.
#
# The suite prj.conf is appended later by opendeck_test_bootstrap(), allowing a
# test to override imported feature flags while keeping the imported target
# shape.
function(opendeck_test_import_target_config)
    list(APPEND CONF_FILE "${opendeck_target_config}")

    opendeck_read_app_bool(target_host_test_enabled CONFIG_PROJECT_TARGET_SUPPORT_HOST_TEST)
    opendeck_read_app_bool(target_hardware_test_enabled CONFIG_PROJECT_TARGET_SUPPORT_HARDWARE_TEST)

    if("${OPENDECK_TEST_MODE}" STREQUAL "host")
        if(NOT target_host_test_enabled STREQUAL "y")
            message(FATAL_ERROR "Target '${OPENDECK_TARGET}' is disabled for host tests in ${opendeck_app_config}.")
        endif()
    elseif("${OPENDECK_TEST_MODE}" STREQUAL "hardware")
        if(NOT target_hardware_test_enabled STREQUAL "y")
            message(FATAL_ERROR "Target '${OPENDECK_TARGET}' is disabled for hardware tests in ${opendeck_app_config}.")
        endif()
    else()
        message(FATAL_ERROR "Unknown OPENDECK_TEST_MODE '${OPENDECK_TEST_MODE}'. Expected 'host' or 'hardware'.")
    endif()

    set(CONF_FILE "${CONF_FILE}" PARENT_SCOPE)
endfunction()
