set(PORT $ENV{PORT})
set(PROBE_ID $ENV{PROBE_ID})
set(FLASH_TOOL $ENV{FLASH_TOOL})
set(FLASH $ENV{FLASH})
set(FLASH_BINARY_DIR $ENV{FLASH_BINARY_DIR})

if (NOT FLASH_BINARY_DIR)
    set(FLASH_BINARY_DIR ${CMAKE_BINARY_DIR})
endif()

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(FLASHING_SCRIPT ${PROJECT_ROOT}/scripts/flashing/${FLASH_TOOL}.sh)

if (EXISTS ${FLASHING_SCRIPT})
    execute_process(
        COMMAND ${FLASHING_SCRIPT} ${TARGET}
        WORKING_DIRECTORY ${FLASH_BINARY_DIR}
        TIMEOUT 90
        RESULT_VARIABLE flash_result
    )

    if(NOT ${flash_result} EQUAL "0")
        message(FATAL_ERROR "Flashing failed")
    endif()
else()
    message(FATAL_ERROR "Provided flash tool doesn't exist or not specified.")
endif()