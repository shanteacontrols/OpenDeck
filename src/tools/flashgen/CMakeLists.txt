cmake_minimum_required(VERSION 3.22)
set(TARGET "" CACHE STRING "Target name")

project(flashgen)
enable_language(CXX)

include($ENV{ZEPHYR_PROJECT}/cmake/include/helpers.cmake)
include($ENV{ZEPHYR_WS}/modules/libcore/cmake/ccache.cmake)
include($ENV{ZEPHYR_PROJECT}/src/generated/target/${TARGET}/CMakeLists.txt)

add_executable(flashgen)
add_module(libemueeprom)
add_module(liblessdb)

target_compile_definitions(libemueeprom
    PUBLIC
    EMU_EEPROM_PAGE_SIZE=${EMU_EEPROM_PAGE_SIZE}
)

target_include_directories(libemueeprom
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/src/generated/mcu/${PROJECT_TARGET_MCU}
    $ENV{ZEPHYR_WS}/modules/libsysexconf/include
    $ENV{ZEPHYR_WS}/modules/libmidi/include
)

target_compile_definitions(flashgen
    PUBLIC
    ${PROJECT_TARGET_DEFINES}
    PROJECT_MCU_FLASH_PAGE_FACTORY=${PROJECT_MCU_FLASH_PAGE_FACTORY}
)

target_include_directories(flashgen
    PUBLIC
    $ENV{ZEPHYR_PROJECT}/src/firmware
    $ENV{ZEPHYR_WS}/modules/u8g2/csrc
    $ENV{ZEPHYR_WS}/modules/libcore/include
)

file(GLOB_RECURSE APP_GENERATED_SOURCES
    $ENV{ZEPHYR_PROJECT}/src/generated/application/${TARGET}/*.cpp
)

target_sources(flashgen
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    $ENV{ZEPHYR_PROJECT}/src/firmware/application/database/custom_init.cpp
    $ENV{ZEPHYR_PROJECT}/src/firmware/application/database/database.cpp
    $ENV{ZEPHYR_PROJECT}/src/firmware/application/util/configurable/configurable.cpp
    ${APP_GENERATED_SOURCES}
)

target_link_libraries(flashgen
    PUBLIC
    libemueeprom
    liblessdb
    mcu
)