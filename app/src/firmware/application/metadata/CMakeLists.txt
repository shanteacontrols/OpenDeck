add_library(metadata INTERFACE)

if(NOT DEFINED PRESET_NAME OR "${PRESET_NAME}" STREQUAL "")
    execute_process(
        COMMAND dasel -p yaml --plain -f $ENV{ZEPHYR_PROJECT}/app/presets.yml presets.[0].name
        OUTPUT_VARIABLE PRESET_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

execute_process(
    COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1"
    WORKING_DIRECTORY $ENV{ZEPHYR_PROJECT}/app
    OUTPUT_VARIABLE SW_VERSION_MAJOR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2"
    WORKING_DIRECTORY $ENV{ZEPHYR_PROJECT}/app
    OUTPUT_VARIABLE SW_VERSION_MINOR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3"
    WORKING_DIRECTORY $ENV{ZEPHYR_PROJECT}/app
    OUTPUT_VARIABLE SW_VERSION_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND $ENV{ZEPHYR_PROJECT}/scripts/gen/gen_fw_uid.sh ${PRESET_NAME}
    WORKING_DIRECTORY $ENV{ZEPHYR_PROJECT}/app
    OUTPUT_VARIABLE PROJECT_TARGET_UID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(COMMAND zephyr_compile_definitions)
    zephyr_compile_definitions(
        SW_VERSION_MAJOR=${SW_VERSION_MAJOR}
        SW_VERSION_MINOR=${SW_VERSION_MINOR}
        SW_VERSION_REVISION=${SW_VERSION_REVISION}
        PROJECT_TARGET_UID=${PROJECT_TARGET_UID}
    )
endif()

target_compile_definitions(metadata
    INTERFACE
    SW_VERSION_MAJOR=${SW_VERSION_MAJOR}
    SW_VERSION_MINOR=${SW_VERSION_MINOR}
    SW_VERSION_REVISION=${SW_VERSION_REVISION}
    PROJECT_TARGET_UID=${PROJECT_TARGET_UID}
)
