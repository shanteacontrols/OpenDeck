string(REPLACE " " ";" FIRMWARE_HEX_LIST "${FIRMWARE_HEX_LIST}")
set(index 0)
message(STATUS "Merging all firmware binary files into a single hex file")

foreach(item IN LISTS FIRMWARE_HEX_LIST)
    message(STATUS "Appending: ${item}")

    if (${index} EQUAL "0")
        execute_process(
            COMMAND cp ${item} ${FIRMWARE_MERGED_HEX}
        )
    else()
        execute_process(
            COMMAND srec_cat ${FIRMWARE_MERGED_HEX} -Intel ${item} -Intel -o ${FIRMWARE_MERGED_HEX} -Intel
        )
    endif()

    math(EXPR current_index "${index} + 1")
    math(EXPR index "${index} + 1")
endforeach()

message(STATUS "Converting merged hex file to bin")

execute_process(
    COMMAND objcopy -I ihex ${FIRMWARE_MERGED_HEX} --gap-fill 0xFF -O binary ${FIRMWARE_MERGED_BIN}
)