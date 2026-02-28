cmake_minimum_required(VERSION 3.22)

set(WORKSPACE_ROOT $ENV{WORKSPACE_DIR})
set(PROJECT_ROOT ${WORKSPACE_ROOT}/OpenDeck)

execute_process(
    COMMAND
    "${PROJECT_ROOT}/scripts/gen/gen_target.sh"
    "--project=OpenDeck"
    "--target-config=${PROJECT_ROOT}/config/target/${TARGET}.yml"
    "--gen-dir-target=${PROJECT_ROOT}/src/generated/target/${TARGET}"
    "--touchscreen-config=${PROJECT_ROOT}/config/touchscreen/${TARGET}.json"
    "--gen-dir-touchscreen=${PROJECT_ROOT}/src/generated/application/${TARGET}"
    "--base-gen-dir-mcu=${PROJECT_ROOT}/src/generated/mcu"
)

include(${PROJECT_ROOT}/cmake/include/helpers.cmake)
include(${PROJECT_ROOT}/src/generated/target/${TARGET}/CMakeLists.txt)

project(firmware)
enable_language(C CXX ASM)

add_subdirectory(board)

if("PROJECT_TARGET_USB_OVER_SERIAL_HOST" IN_LIST PROJECT_TARGET_DEFINES)
    add_subdirectory(usb-link)
else()
    add_subdirectory(application)
endif()