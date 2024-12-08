cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

project(flashgen)
enable_language(CXX)

set(WORKSPACE_ROOT $ENV{WORKSPACE_DIR})
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../)

include(${PROJECT_ROOT}/cmake/include/helpers.cmake)
include(${WORKSPACE_ROOT}/modules/libcore/cmake/ccache.cmake)
include(${PROJECT_ROOT}/src/generated/target/${TARGET}/CMakeLists.txt)

add_executable(flashgen)
add_module(libemueeprom)
add_module(liblessdb)

target_compile_definitions(libemueeprom
    PUBLIC
    EMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
)

target_include_directories(libemueeprom
    PUBLIC
    ${PROJECT_ROOT}/src/generated/mcu/${PROJECT_TARGET_MCU}
    ${WORKSPACE_ROOT}/modules/libsysexconf/include
    ${WORKSPACE_ROOT}/modules/libmidi/include
)

target_compile_definitions(flashgen
    PUBLIC
    ${PROJECT_TARGET_DEFINES}
    PROJECT_MCU_FLASH_PAGE_FACTORY=${PROJECT_MCU_FLASH_PAGE_FACTORY}
)

target_include_directories(flashgen
    PUBLIC
    ${PROJECT_ROOT}/src/firmware
    ${WORKSPACE_ROOT}/modules/u8g2/csrc
    ${WORKSPACE_ROOT}/modules/libcore/include
)

file(GLOB_RECURSE APP_GENERATED_SOURCES
    ${PROJECT_ROOT}/src/generated/application/${TARGET}/*.cpp
)

target_sources(flashgen
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${PROJECT_ROOT}/src/firmware/application/database/custom_init.cpp
    ${PROJECT_ROOT}/src/firmware/application/database/database.cpp
    ${PROJECT_ROOT}/src/firmware/application/util/configurable/configurable.cpp
    ${APP_GENERATED_SOURCES}
)

target_link_libraries(flashgen
    PUBLIC
    libemueeprom
    liblessdb
    mcu
)