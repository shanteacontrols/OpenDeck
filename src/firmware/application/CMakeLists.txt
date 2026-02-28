add_subdirectory(database)
add_subdirectory(io)
add_subdirectory(protocol)
add_subdirectory(system)
add_subdirectory(util)

add_executable(application)

execute_process(
    COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f1"
    OUTPUT_VARIABLE SW_VERSION_MAJOR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f2"
    OUTPUT_VARIABLE SW_VERSION_MINOR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND sh "-c" "git describe --tags --abbrev=0 | cut -c 2- | cut -d. -f3"
    OUTPUT_VARIABLE SW_VERSION_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_include_directories(application
    PRIVATE
    ${PROJECT_ROOT}/src/firmware
)

add_library(version INTERFACE)

target_compile_definitions(version
    INTERFACE
    SW_VERSION_MAJOR=${SW_VERSION_MAJOR}
    SW_VERSION_MINOR=${SW_VERSION_MINOR}
    SW_VERSION_REVISION=${SW_VERSION_REVISION}
)

file(GLOB_RECURSE APP_GENERATED_SOURCES
    ${PROJECT_ROOT}/src/generated/application/${TARGET}/*.cpp
)

add_module(liblessdb)
add_mcu_library(liblessdb)

add_module(libsysexconf)
add_mcu_library(libsysexconf)

add_module(libmidi)
add_mcu_library(libmidi)

add_mcu_library(database)
add_mcu_library(util)
add_mcu_library(midi)
add_mcu_library(analog)
add_mcu_library(buttons)
add_mcu_library(encoders)
add_mcu_library(i2c)
add_mcu_library(leds)
add_mcu_library(touchscreen)
add_mcu_library(system)

target_sources(application
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${APP_GENERATED_SOURCES}
)

target_link_libraries(application
    PRIVATE
    ${BOARD_APPLICATION_LIBRARIES}
    liblessdb
    libsysexconf
    libmidi
    database
    util
    midi
    analog
    buttons
    encoders
    i2c
    version
    leds
    touchscreen
    system
)

add_custom_command(
    TARGET application
    POST_BUILD
    COMMAND echo Converting elf file to hex
    COMMAND objcopy --gap-fill 0xFF -O ihex $<TARGET_FILE:application> $<TARGET_FILE:application>.hex
    COMMAND echo Appending firmware metadata to hex file
    COMMAND srec_cat $<TARGET_FILE:application>.hex -Intel -exclude ${PROJECT_MCU_FLASH_ADDR_FW_METADATA_START} ${PROJECT_MCU_FLASH_ADDR_FW_METADATA_END} -MAximum_Little_Endian ${PROJECT_MCU_FLASH_ADDR_FW_METADATA_START} -o $<TARGET_FILE:application>.hex -Intel
    COMMAND echo Adding firmware CRC at the end of application flash
    COMMAND srec_cat $<TARGET_FILE:application>.hex -Intel -Cyclic_Redundancy_Check_16_Little_Endian -MAximum-Address $<TARGET_FILE:application>.hex -Intel -Cyclic_Redundancy_Check_16_XMODEM -Output $<TARGET_FILE:application>.hex -Intel
    COMMAND echo Converting hex file to bin
    COMMAND objcopy -I ihex -O binary $<TARGET_FILE:application>.hex $<TARGET_FILE:application>.bin
)

list(APPEND FIRMWARE_HEX_LIST
    $<TARGET_FILE:application>.hex
)