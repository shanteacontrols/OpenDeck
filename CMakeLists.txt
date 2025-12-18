cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

set(PROJECT_NAME "OpenDeck")
set(WORKSPACE_ROOT $ENV{WORKSPACE_DIR})
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(FIRMWARE_TARGETS "")
set(FIRMWARE_HEX_LIST "")
set(FIRMWARE_MERGED_HEX ${CMAKE_CURRENT_BINARY_DIR}/merged.hex)
set(FIRMWARE_MERGED_BIN ${CMAKE_CURRENT_BINARY_DIR}/merged.bin)
set(FIRMWARE_MERGED_UF2 ${CMAKE_CURRENT_BINARY_DIR}/merged.uf2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(ExternalProject)
include(${PROJECT_ROOT}/cmake/include/helpers.cmake)

# Needed for tools and tests
execute_process(
    COMMAND
    "${PROJECT_ROOT}/scripts/gen/gen_mcu.sh"
    "--mcu=stub"
    "--gen-dir=${PROJECT_ROOT}/src/generated/mcu/stub"
)

execute_process(
    COMMAND
    "${PROJECT_ROOT}/scripts/gen/gen_target.sh"
    "--project=${PROJECT_NAME}"
    "--target-config=${PROJECT_ROOT}/config/target/${TARGET}.yml"
    "--gen-dir-target=${PROJECT_ROOT}/src/generated/target/${TARGET}"
    "--touchscreen-config=${PROJECT_ROOT}/config/touchscreen/${TARGET}.json"
    "--gen-dir-touchscreen=${PROJECT_ROOT}/src/generated/application/${TARGET}"
    "--base-gen-dir-mcu=${PROJECT_ROOT}/src/generated/mcu"
)

include(${PROJECT_ROOT}/src/firmware/board/CMakeLists.txt)
include(${PROJECT_ROOT}/src/firmware/bootloader/CMakeLists.txt)
include(${PROJECT_ROOT}/src/firmware/application/CMakeLists.txt)
include(${PROJECT_ROOT}/src/firmware/usb-link/CMakeLists.txt)

project(${PROJECT_NAME})
enable_language(C CXX ASM)

add_custom_command(
    DEPENDS ${FIRMWARE_TARGETS} # ensure re-merge once all firmware targets change
    OUTPUT ${FIRMWARE_MERGED_HEX}
    OUTPUT ${FIRMWARE_MERGED_BIN}
    COMMAND ${CMAKE_COMMAND} -DFIRMWARE_MERGED_HEX=${FIRMWARE_MERGED_HEX} -DFIRMWARE_MERGED_BIN=${FIRMWARE_MERGED_BIN} -DFIRMWARE_HEX_LIST="${FIRMWARE_HEX_LIST}" -P ${PROJECT_ROOT}/cmake/scripts/merge_fw_binaries.cmake
)

add_custom_target(merge_firmware_binaries
    ALL
    DEPENDS ${FIRMWARE_MERGED_HEX}
)

add_dependencies(merge_firmware_binaries ${FIRMWARE_TARGETS})

if(CORE_MCU_UF2_ID)
    add_custom_command(
        DEPENDS ${FIRMWARE_MERGED_BIN}
        OUTPUT ${FIRMWARE_MERGED_UF2}
        COMMAND ${WORKSPACE_ROOT}/modules/tinyusb/tools/uf2/utils/uf2conv.py -b ${CORE_MCU_FLASH_START_ADDR} -f ${CORE_MCU_UF2_ID} -o ${FIRMWARE_MERGED_UF2} -c ${FIRMWARE_MERGED_BIN}
    )

    add_custom_target(create_uf2_binary
        ALL
        DEPENDS ${FIRMWARE_MERGED_UF2}
    )

    add_dependencies(create_uf2_binary merge_firmware_binaries)
endif()

add_custom_target(flash
    COMMAND ${CMAKE_COMMAND} -DTARGET=${TARGET} -P ${PROJECT_ROOT}/cmake/scripts/flash.cmake
)

set_target_properties(flash PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_dependencies(flash merge_firmware_binaries)

add_custom_target(format
    COMMAND echo Checking code formatting...
    COMMAND ${PROJECT_ROOT}/scripts/code_format.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set_target_properties(format PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_custom_target(lint
    COMMAND ${PROJECT_ROOT}/scripts/clang_tidy_wrapper.sh --mcu=${PROJECT_TARGET_MCU} --output=${CMAKE_CURRENT_BINARY_DIR}/clang-tidy-fixes.yml --compiler=${CMAKE_C_COMPILER} --option-files-dir=${PROJECT_ROOT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(lint PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_dependencies(lint merge_firmware_binaries)

ExternalProject_Add(tests
    SOURCE_DIR      ${PROJECT_ROOT}/tests
    CMAKE_ARGS      -DTARGET=${TARGET}
    CMAKE_ARGS      -DCMAKE_USE_STUB_MCU=1
    CMAKE_ARGS      -DFW_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS      -DCMAKE_BUILD_TYPE=Debug
    BINARY_DIR      ${CMAKE_CURRENT_BINARY_DIR}/tests
    BUILD_ALWAYS    TRUE
    INSTALL_COMMAND ""
)

add_dependencies(tests merge_firmware_binaries)
set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL True)